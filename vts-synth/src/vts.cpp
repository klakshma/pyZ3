#include<vts.h>
#include<z3-util.h>

// define n dimentional vectors
//#include "boost/multi_array.hpp"
//#include <cassert>

// Define the value of the num_nodes, total_molecules etc.
// #define n 2    // num of nodes.
// #define m 2    // num of molecules.
// #define q 1    // num of parellel edges.
// #define v 1    // model variation.
// #define c 2    // connectivity.

// Create a 3D array that is 3 x 4 x 2
//typedef boost::multi_array<z3::expr, 3> array_type;
//typedef array_type::index index;
//array_type A(boost::extents[3][4][2]);

//============================================================================
// Populates the vectors with z3 variables

void vts::popl1 ( VecExpr& m, unsigned arg1,
                  std::string prefix ) {
  for ( unsigned int i = 0; i < arg1; i++ ) {
    std::string name = prefix + "_" + std::to_string(i);
    m.push_back( make_bool( ctx, name ) );
  }
}

void vts::popl2 ( Vec2Expr& m, unsigned arg1, unsigned arg2,
                  std::string prefix) {
  m.resize(arg1);
  for ( unsigned int i = 0; i < arg1; i++ )
    popl1( m[i], arg2, prefix+"_"+ std::to_string(i) );
}

void vts::popl3 ( Vec3Expr& m, unsigned arg1, unsigned arg2,
                  unsigned arg3, std::string prefix) {
  m.resize(arg1);
  for ( unsigned int i = 0; i < arg1; i++ )
    popl2( m[i], arg2, arg3, prefix+"_"+ std::to_string(i) );
}

void vts::popl4 ( Vec4Expr& m, unsigned arg1, unsigned arg2,
                  unsigned arg3, unsigned arg4, std::string prefix) {
  m.resize(arg1);
  for ( unsigned int i = 0; i < arg1; i++ )
    popl3( m[i], arg2, arg3, arg4, prefix+"_"+ std::to_string(i) );
}

void vts::init_vts() {

// Populate edges: e(i,j,q)
  popl3( edges, N, N, E_arity, "e" );

// Populate nodes: n(i,j)
  popl2 ( nodes, N, M, "n" );

// Populate active_node (i, k)
  popl2 ( active_node, N, M, "a" );

// Populate presence_edge(i,j,q,k)
  popl4 ( presence_edge, N, N, E_arity, M, "e" );

// Populate active_edge(i,j,q,k)
  popl4 ( active_edge, N, N, E_arity, M, "b" );

// Populate pairing_m(k,k1)
  popl2 ( pairing_m, M, M, "p" );

// Populate node_funcs : Currently not handled.
// Populate edge_funcs : Currently not handled.

// Populate reach(i,j,k,z)
  popl4 ( reach, N, N, M, N-1, "r" );

// Populate drop1(i,j,q)
  popl3 ( drop1, N, N, E_arity, "d1");

//Populate d_reach(i,j)
  popl2 ( d_reach, N , N, "r1" );


}

z3::expr vts::is_mol_edge_present( unsigned i, unsigned j, unsigned m ) {
  z3::expr_vector p_list(ctx);
  for( unsigned q = 0; q < E_arity ; q++ ) {
    p_list.push_back( presence_edge[i][j][q][m] );
  }
  return z3::mk_or( p_list );
}

z3::expr vts::reachability_def() {
  z3::expr_vector a_list(ctx);
  for( unsigned i = 0; i < N; i++ ) {
    for( unsigned j = 0; j < N; j++ ) {
      if( i == j ) continue;
      for( unsigned m = 0; m < M; m++ ) {
        for( unsigned z = 0; z < N-1; z++ ) {
          if( z ==0 ) {
            a_list.push_back( z3::implies( reach[i][j][m][0], is_mol_edge_present(i,j,m)) );
          }else{
            z3::expr_vector rhs_list(ctx);
            for( unsigned l = 0; l < N; l++ ) {
              if( l == i || l == j ) continue;
              rhs_list.push_back( reach[l][j][m][z-1] && is_mol_edge_present(i,l,m) );
            }
            auto rhs = z3::mk_or( rhs_list );
            a_list.push_back( z3::implies( reach[i][j][m][z], rhs ) );
          }
        }
      }
    }
  }
  return z3::mk_and( a_list );
}

//V1: For an edge to exist it should have one molecule present.
z3::expr vts::molecule_presence_require_for_present_edge() {
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        z3::expr rhs = ctx.bool_val(false); 
        for ( unsigned k = 0; k < M; k++ ) {
          rhs = presence_edge[i][j][q][k] || rhs;
        }
        ls.push_back( implies (rhs, edges[i][j][q]) );
      }
    }
  }
  return z3::mk_and( ls );
}

// V2: If molecule is active on an edge then it should be present on the edge.
z3::expr vts::active_molecule_is_present_on_edge() {         //V2
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)  
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        for ( unsigned k = 0; k < M; k++ ) {
          z3::expr e =  implies( active_edge[i][j][q][k] , presence_edge[i][j][q][k] );
          ls.push_back( e );
        }
      }
    }
  }
  return z3::mk_and( ls );
}


// V3: A mmolecule should be present to be active.
z3::expr vts::active_molecule_is_present_on_node() {         //V3
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned k = 0 ; k < M; k++ ) {
      z3::expr e = z3::implies( active_node[i][k], nodes[i][k] );
      ls.push_back ( e );
    }
  }
  return z3::mk_and( ls );
}

// V4: The edge label are subset of the source and target.
z3::expr vts::edge_modelecues_is_subset_of_node_molecules() { //V4
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)  
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        for( unsigned k = 0 ; k < M; k++ ) {
          z3::expr e = z3::implies( presence_edge[i][j][q][k], nodes[i][k] && nodes[j][k] );
        }
      }
    }
  }
  return z3::mk_and( ls );
}


// V5 : that no self edges are allowed. 
z3::expr vts::no_self_edges() {                              //V5
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned q = 0 ; q < E_arity; q++ ) {
      ls.push_back( !edges[i][i][q] );
    }
  }
  return z3::mk_and( ls );
}

// V6: Only Q R entry has possible non zero entry.  

z3::expr vts::restriction_on_pairing_matrix() {              //V6
  z3::expr_vector ls(ctx);
  for( unsigned x = 0 ; x < M; x++ ) {
    for( unsigned y = 0 ; y < M; y++ ) {
      if ( ((x < M/2) && (y < M/2)) || ((x>=M/2) && (y >=M/2)) ) {
        z3::expr e = !pairing_m[x][y];
        ls.push_back( e );
      }
    }
  }
  return z3::mk_and( ls );
}


// V7 : There should be an active pair corresponding to pairing matrix 
z3::expr vts::edge_must_fuse_with_target() {                 //V7
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)  
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        z3::expr lhs = c.bool_val(false); 
        for ( unsigned k = 0; k < M; k++ ) {
          for ( unsigned k1 = 0; k1 < M; k1++ ) {
            if (k == k1) 
              continue;
            lhs =  ( (active_edge[i][j][q][k] && active_node[j][k1] && p[k][k1]) ||  lhs );
          }
        }
        z3::expr e =  implies ( edge[i][j][q], lhs );
        ls.push_back( e );
      }
    }
  }
  return z3::mk_and( ls );
}

//  V8: Edge should not be able to potentially fuse with 
//      any node other than it's target.
z3::expr vts::edge_must_not_fuse_with_noone_else() {       //V8
  z3::expr_vector ls(ctx);
  for( unsigned i = 0 ; i < N; i++ ) {
    for( unsigned j = 0 ; j < N; j++ ) {
      if (j == i)  
        continue;
      for ( unsigned q = 0; q < E_arity; q++ ) {
        for ( unsigned k = 0; k < M; k++ ) {
          z3::expr lhs = c.bool_val(false); 
            for( unsigned j1 = 0 ; j1 < N; j1++ ) {
              if (j1 == j) 
                continue;
              for ( unsigned k11 = 0; k11 < M; k11++ ) {
              if (k == k11) 
                continue;
              lhs =  ( ( active_node[j1][k11] && p[k][k11] ) ||  lhs );
              }
            }
            z3::expr e = implies( active_edge[i][j][q][k], !lhs );
            ls.push_back( e );
        }
      }
    }
  }
  return z3::mk_and( ls );
}




}
