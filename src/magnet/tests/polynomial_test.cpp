#define BOOST_TEST_MODULE Polynomial_test
#include <boost/test/included/unit_test.hpp>
#include <magnet/math/polynomial.hpp>
#include <magnet/math/vector.hpp>
#include <cmath>

bool err(double val, double expected)
{
  return std::abs(val / expected - 1) < 0.0001;
}

BOOST_AUTO_TEST_CASE( poly_addition )
{
  using namespace magnet::math;
  Polynomial<1> x{0, 2.5};
  Polynomial<0> C{0.3};
  auto poly1 = x+C;
  BOOST_CHECK_EQUAL(poly1[0], 0.3);
  BOOST_CHECK_EQUAL(poly1[1], 2.5);

  auto poly2 = x + 0.3;
  BOOST_CHECK_EQUAL(poly2[0], 0.3);
  BOOST_CHECK_EQUAL(poly2[1], 2.5);
}

BOOST_AUTO_TEST_CASE( poly_multiplication )
{
  using namespace magnet::math;
  Polynomial<1> x{0, 1};
  auto poly1 = -2.0;
  auto poly2 = 2.0 - x + x * x;
  auto poly3 = poly2 * poly1;
  BOOST_CHECK_EQUAL(poly3[0], -4);
  BOOST_CHECK_EQUAL(poly3[1], +2);
  BOOST_CHECK_EQUAL(poly3[2], -2);
}

BOOST_AUTO_TEST_CASE( poly_vector )
{
  using namespace magnet::math;
  Polynomial<1, Vector> x{Vector(), Vector{1,2,3}};
  Polynomial<0, Vector> C{Vector{3,2,1}};
  auto poly1 = x+C;
  BOOST_CHECK_EQUAL(poly1[0], (Vector{3,2,1}));
  BOOST_CHECK_EQUAL(poly1[1], (Vector{1,2,3}));
  
  auto poly2 = poly1 * poly1;
  BOOST_CHECK_EQUAL(poly2[0], 14);
  BOOST_CHECK_EQUAL(poly2[1], 20);
  BOOST_CHECK_EQUAL(poly2[2], 14);
}

BOOST_AUTO_TEST_CASE( poly_lower_order )
{
  using namespace magnet::math;
  Polynomial<1> x{0, 1};
  Polynomial<2> poly2 = 2.0 - x + x * x;
  Polynomial<3> poly3 = poly2 + 0 * x * x * x;
  //Try to cast down one level as the highest order coefficient is zero
  BOOST_CHECK(poly3[3] == 0);
  Polynomial<2> poly4(poly3);

  BOOST_CHECK_EQUAL(poly4[0], 2);
  BOOST_CHECK_EQUAL(poly4[1], -1);
  BOOST_CHECK_EQUAL(poly4[2], 1);
  BOOST_CHECK_EQUAL(poly3(123), poly4(123));
}

BOOST_AUTO_TEST_CASE( poly_derivative )
{
  using namespace magnet::math;
  Polynomial<1> x{0, 1};
  auto poly1 = x + x*x + x*x*x + x*x*x*x;
  auto poly2 = derivative(poly1);
  BOOST_CHECK_EQUAL(poly2[0], 1);
  BOOST_CHECK_EQUAL(poly2[1], 2);
  BOOST_CHECK_EQUAL(poly2[2], 3);  
  BOOST_CHECK_EQUAL(poly2[3], 4);  

  auto poly3 = 2.0 - x + 2 * x * x;
  auto poly4 = derivative(poly3);
  BOOST_CHECK_EQUAL(poly4[0], -1);
  BOOST_CHECK_EQUAL(poly4[1], 4);
  BOOST_CHECK_EQUAL(poly4(0), -1);
  BOOST_CHECK_EQUAL(poly4(1), 3);
}

BOOST_AUTO_TEST_CASE( poly_zero_derivative)
{
  using namespace magnet::math;
  const Polynomial<1> x{0, 1};
  const auto poly1 = derivative(x);
  BOOST_CHECK_EQUAL(poly1[0], 1);

  const auto poly2 = derivative(poly1);
  BOOST_CHECK_EQUAL(poly2[0], 0);

  const auto poly3 = derivative(poly2);
  BOOST_CHECK_EQUAL(poly3[0], 0);

  const auto poly4 = derivative(poly3);
  BOOST_CHECK_EQUAL(poly4[0], 0);
}

BOOST_AUTO_TEST_CASE( poly_quadratic_roots)
{
  using namespace magnet::math;
  Polynomial<1> x{0, 1};
  
  {//Quadratic with catastrophic cancellation of error
    auto poly = x * x + 712345.12 * x + 1.25;  
    auto roots = solve_roots(poly);
    BOOST_CHECK(roots.size() == 2);

    std::sort(roots.begin(), roots.end());
    
    BOOST_CHECK_CLOSE(roots[0], -712345.1199985961, 1e-10);
    BOOST_CHECK_CLOSE(roots[1], -1.754767408250742e-6, 1e-10);
  }

  {//Quadratic with no roots
    auto poly = x * x - 3 * x + 4;
    auto roots = solve_roots(poly);
    BOOST_CHECK(roots.size() == 0);
  }

  {//Quadratic with one root
    auto poly = -4 * x * x + 12 * x - 9;
    auto roots = solve_roots(poly);
    BOOST_CHECK(roots.size() == 1);
    
    BOOST_CHECK_CLOSE(roots[0], 1.5, 1e-10);
  }
  
  {//linear function with one root
    auto poly =  0 * x * x + 12 * x - 9;
    auto roots = solve_roots(poly);
    BOOST_CHECK(roots.size() == 1);
    
    BOOST_CHECK_CLOSE(roots[0], 0.75, 1e-10);
  }

  {//constant function, with no roots
    auto poly =  0 * x * x + 0 * x - 9;
    auto roots = solve_roots(poly);
    BOOST_CHECK(roots.size() == 0);
  }
}
