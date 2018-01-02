#include <stdio.h>
#include <z3++.h>
#include <vector>

#include "z3-util.h"

int main() {

    z3::context c;

    z3::expr t = c.bool_val( true  );
    z3::expr fal = c.bool_val( false );

    z3::expr x = c.bool_const("x");
    z3::expr y = c.bool_const("y");
    z3::expr z = c.bool_const("z");
    z3::expr w = c.bool_const("w");
    z3::expr f = exists( x, forall( z, x && z && forall( y, exists( w, implies( y, w) && x && z) ) ) );
   // z3::expr f = forall(x, exists( y, (x == !y )) ) ;
  //z3::expr f = forall( x, exists( w, w && forall ( y,  x&& y ) ) ) ;
    // z3::expr f =  t;
   //z3::expr f = forall( x, forall ( y, exists (z,  z == x||  z == y) )) ; 
    //std::cout << "The sort of the formula f is: " << Z3_get_sort( c, f ) << "\n";
    //exit(0);

    std::vector<z3::expr_vector> qs;
    auto prenex_f = prenex( f, qs );

    std::cout << "Prenexed f : " << prenex_f << "\n";

    bool fall = false;
    for( auto& q : qs ) {
      if( fall ) {
        std::cout << "forall " << q << "\n";
      }else{
        std::cout << "exists " << q << "\n";
      }
      fall = !fall;
    }

    z3::solver s(c);
    z3::goal g(c);
    g.add( f );

    z3::tactic t1(c, "qsat");
    // // tactic t2(c, "solve-eqs");
    // tactic t = t1 & t2;
    z3::apply_result r = t1(g);
    //std::cout << r << "\n";

  return 0;
}
