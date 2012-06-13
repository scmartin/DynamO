/*  dynamo:- Event driven molecular dynamics simulator 
    http://www.dynamomd.org
    Copyright (C) 2011  Marcus N Campbell Bannerman <m.bannerman@gmail.com>

    This program is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 3 as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <dynamo/simulation.hpp>
#include <dynamo/include.hpp>
#include <dynamo/schedulers/scheduler.hpp>
#include <dynamo/include.hpp>
#include <dynamo/interactions/intEvent.hpp>
#include <dynamo/outputplugins/outputplugin.hpp>
#include <dynamo/liouvillean/liouvillean.hpp>
#include <dynamo/systems/system.hpp>
#include <dynamo/NparticleEventData.hpp>
#include <dynamo/systems/sysTicker.hpp>
#include <dynamo/globals/ParabolaSentinel.hpp>
#include <magnet/exception.hpp>
#include <boost/foreach.hpp>
#include <iomanip>

#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/chain.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/filesystem.hpp>

namespace dynamo {
  void 
  Simulation::setTickerPeriod(double nP)
  {
    SysTicker* ptr = dynamic_cast<SysTicker*>(&systems["SystemTicker"]);
    if (ptr == NULL)
      M_throw() << "Could not find system ticker (maybe not required?)";

    ptr->setTickerPeriod(nP * units.unitTime());
  }

  void 
  Simulation::scaleTickerPeriod(double nP)
  {
    SysTicker* ptr = dynamic_cast<SysTicker*>(&systems["SystemTicker"]);

    if (ptr == NULL)
      M_throw() << "Could not find system ticker (maybe not required?)";

    ptr->setTickerPeriod(nP * ptr->getPeriod());
  }

  void 
  Simulation::addOutputPlugin(std::string Name)
  {
    if (status >= INITIALISED)
      M_throw() << "Cannot add plugins now";
  
    dout << "Loading output plugin string " << Name << std::endl;

    shared_ptr<OutputPlugin> tempPlug(OutputPlugin::getPlugin(Name, this));
    outputPlugins.push_back(tempPlug);
  }

  void 
  Simulation::simShutdown()
  { nextPrintEvent = endEventCount = eventCount; }

  void
  Simulation::runSimulation(bool silentMode)
  {
    if (status != INITIALISED && status != PRODUCTION)
      M_throw() << "Bad state for runSimulation()";

    status = PRODUCTION;

    size_t nextPrint = eventCount + eventPrintInterval;

    while (eventCount < endEventCount)
      try
	{
	  ptrScheduler->runNextEvent();
	
	  //Periodic work
	  if ((eventCount >= nextPrint) && !silentMode && outputPlugins.size())
	    {
	      //Print the screen data plugins
	      BOOST_FOREACH(shared_ptr<OutputPlugin> & Ptr, outputPlugins)
		Ptr->periodicOutput();
	      
	      nextPrint = eventCount + eventPrintInterval;
	      std::cout << std::endl;
	    }
	}
      catch (std::exception &cep)
	{
	  M_throw() << "\nWhile executing event "
		    << eventCount << cep.what();
	}
  }

  void 
  Simulation::configLoaded()
  {
    //Handled by an input plugin
    if (status != START)
      M_throw() << "Loading config at wrong time";
  
    status = CONFIG_LOADED;
  }

  void
  Simulation::outputData(std::string filename)
  {
    if (status < INITIALISED || status == ERROR)
      M_throw() << "Cannot output data when not initialised!";

    namespace io = boost::iostreams;
    io::filtering_ostream coutputFile;
  
    if (std::string(filename.end()-4, filename.end()) == ".bz2")
      coutputFile.push(io::bzip2_compressor());
  
    coutputFile.push(io::file_sink(filename));
  
    magnet::xml::XmlStream XML(coutputFile);
    XML.setFormatXML(true);
  
    XML << std::setprecision(std::numeric_limits<double>::digits10)
	<< magnet::xml::prolog() << magnet::xml::tag("OutputData");
  
    //Output the data and delete the outputplugins
    BOOST_FOREACH(shared_ptr<OutputPlugin> & Ptr, outputPlugins)
      Ptr->output(XML);
  
    XML << magnet::xml::endtag("OutputData");

    dout << "Output written to " << filename << std::endl;
  }

  long double 
  Simulation::getSysTime()
  { return dSysTime / units.unitTime(); }
}