#include <stdio.h>
#include <stdlib.h>

#define M 4
#define N 4
#define snareLength 4
#define dLen 8   // 2 * M  
#define bigLen 256 // 2 ^ (2*M) 
#define len 8


_Bool nondet_bool();
unsigned int nondet_uint();

typedef unsigned __CPROVER_bitvector[M] bitvector; 
typedef unsigned __CPROVER_bitvector[snareLength] snareVector; 
typedef unsigned __CPROVER_bitvector[dLen] dvector;
typedef unsigned __CPROVER_bitvector[bigLen] bigvector;


unsigned int  nondet (){  
  unsigned int num = nondet_uint();
  __CPROVER_assume( num>= 0 && num  <= 1);
   return num;
};

unsigned int zeroTon(unsigned int n) {
    unsigned int result = nondet_uint();
    __CPROVER_assume(result >=0 && result <=n);
    return result ;
};

bigvector nondetBV() {
	     bigvector bee;
	     __CPROVER_assume(bee >= 0b0 && bee <= 0b1111);  
	     return bee;  }


struct EdgeBag
 {
   unsigned int ith;
   unsigned int jth;
   unsigned int count;
   unsigned int count2;
   bitvector edgeWeight;
   snareVector  vSnare;
   snareVector tSnare;
   snareVector combinedMask; 
   snareVector combinedMask2; 
};


int  main()
 {    
	 
    unsigned int pos, i, j, k, l, m ,n ,w, x, y , z, e1,e2,e3, iVal, jVal, g, g0, gl, lastg, ng, nl, nl2 ;
    unsigned int edgePos = 0, bagNo = 0, colorNode = 0 , minColor, cPos = 0 , tComp, result;
    unsigned int  ticks, ticks2, valj, vali , calc, edgeCount = 0;
    _Bool Ck=0, Cl = 0,Cf = 1, C0 = 1, C1 = 1, C2 = 1, C3 = 1, C4, C5, C6 , C7; 

    bigvector b1 = 0b1, b0 = 0b0, vf, vff, edegeInhib[dLen], nodeInhib[dLen];
    dvector bv ,bvv;
   
    bitvector Vnodes[N], Tnodes[N];
    bitvector  fareTotal, inTotal, outTotal , outVSnareTotal , inVSnareTotal , outTSnareTotal , inTSnareTotal ;
    
    snareVector total, cond2Total, cond2fareTotal, centTotal, centTotal2, placeHolder;
    snareVector v, vl, vl2, t, f, h, v2, lastv, lastv2 ,nv, nv2, v0, v02 ;
    snareVector Tedge[N][N], Vedge[N][N] , Vedge2[N][N] , Tedge2[N][N] , fComp , bComp;    
    snareVector qrfusionMatrix[snareLength], rqfusionMatrix[snareLength];     
  
    unsigned int graph[N][N]; 

    for (i = 0; i < N; i++) {
       for (j = 0; j < N; j++) {
	      if(i != j) {
		      __CPROVER_assume(graph[i][j] >= 0 && graph[i][j] <=2);
	          if (graph[i][j] == 1) {
                 edgeCount += 1;	
		      }
              else if (graph[i][j] == 2) {
		         edgeCount += 2;
		      }
          }
          else  
		      __CPROVER_assume(graph[i][j] == 0); 
       } 
    }

   __CPROVER_assume(edgeCount == len);
     
     struct EdgeBag edgeBag[len];

     for  (i = 0; i < N; i++) {
             for  (j = 0; j < N; j++) {
               if ((graph[i][j] == 1) || (graph[i][j] == 2)) {
                   edgeBag[edgePos].ith = i;     // Record the source node
                   edgeBag[edgePos].jth = j;     // Record the target Node
 
                   // Only molecule present at the nodes are allowed to fly out.
                   __CPROVER_assume((edgeBag[edgePos].vSnare  & (~ Vnodes[i])) == 0);
                   __CPROVER_assume((edgeBag[edgePos].tSnare  & (~ Tnodes[i])) == 0); ;
 
                   // Additional Vedge[i][j] and Tedge[i][j] is used to be lookup value in global steady state check condition.
                   Vedge[i][j] = edgeBag[edgePos].vSnare;
                   Tedge[i][j] = edgeBag[edgePos].tSnare;
                   edgePos = edgePos + 1;
             }
 
             if ((graph[i][j] == 2)) {
                  edgeBag[edgePos].ith = i;      // Record the Source Node  
                  edgeBag[edgePos].jth = j;      // Record the Target Node
 
                 // Only molecule present at the nodes are allowed to fly out.
                   __CPROVER_assume((edgeBag[edgePos].vSnare  & (~ Vnodes[i])) == 0);
                   __CPROVER_assume((edgeBag[edgePos].tSnare  & (~ Tnodes[i])) == 0);
                    // Additional Vedge2[i][j] and Tedge2[i][j] is used to be lookup value in global steady state check condition.
                   Vedge2[i][j] = edgeBag[edgePos].vSnare;
                   Tedge2[i][j] = edgeBag[edgePos].tSnare;
                   edgePos = edgePos + 1;
             }

          }
     }
 /* 
     C4 = 0;
         for ( i = 0; i < N ; i++) {
             calc = 0;
             for ( j = 0 ; j < len; j++) {              
                if ( (edgeBag[j].ith == i) || (edgeBag[j].jth == i) ){
                     calc = calc + 1;
                }
               }
             __CPROVER_assume(calc >= 3);
             if(calc < 4) {
                 C4 = 1;
             }
         }
*/
/*
  C5 = 1;

  for (x = 0; x < len; x++) {
     for (y = (x + 1); y < len; y++) {
        for (z = (y + 1); z < len; z++) {
		  
		   unsigned int graph1[N][N];
           
           for (m=0;m<N;m++){
			  for (n=0;n<N;n++) {
		        graph1[m][n] = graph[m][n];
		      }
	        }
	         
	        graph1[edgeBag[x].ith][edgeBag[x].jth] = (graph1[edgeBag[x].ith][edgeBag[x].jth] - 1);
	        graph1[edgeBag[y].ith][edgeBag[y].jth] = (graph1[edgeBag[y].ith][edgeBag[y].jth] - 1);		     
	        graph1[edgeBag[z].ith][edgeBag[z].jth] = (graph1[edgeBag[z].ith][edgeBag[z].jth] - 1);
	*/         
// ------------ working great ------------------------------------------------------------------------//
/*
          // STRONGLY CONNECTED
          for ( i = 0; i < N; i++) {
             for (j = 0; j < N ; j++) {
                if ( (i != j) && ((graph1[i][j] >= 1) || graph1[j][i] >= 1 )) {  // if there is Direct edge we are done
                    C5 = C5 && 1;
                }

                else if(i != j) {  // Else case
                    unsigned int nub;  // Define max hop
                    __CPROVER_assume( nub >= 1 && (nub <= N-2));
                    unsigned int gPath[nub];
                
                    for (k = 0; k < nub; k++) {   // zdynamic N - 2 iteration
                        gPath[k] = zeroTon(N-1);
                     }
                    //  Make sure first edge is connected to i  and last edge is connected to j
                    if( ((graph1[i][gPath[0]] >= 1) || (graph1[gPath[0]][i] >= 1)) 
                    && ((graph1[gPath[nub - 1]][j] >= 1) || (graph1[j][gPath[nub - 1]] >= 1))) {   
                        C5 = C5 && 1;
				    }
                    else  {
                        C5 = 0;
                    }
               // rest Of the case is just checking edge btw consecutive array elements
                 for (l = 0; l < nub - 1; l++) {         //Dynamic N - 3  iteration
                      if ( (graph1[gPath[l]][gPath[l+1]] >= 1 ) || (graph1[gPath[l+1]][gPath[l]] >= 1 )) { 
                               C5 = C5 && 1;
					   }
                      else {
                            C5 = 0;
					  }
                 }
              }   // else if ended 
               
                 
            }
         }
                 
      }
   }
}

      
	for (j = 0; j < len; j++) {   
         C0 = (C0 && (edgeBag[j].vSnare != 0));
         C0 = (C0 && (edgeBag[j].tSnare != 0));
     }

    for ( i = 0; i < N; i++) {
              __CPROVER_assume(Vnodes[i] != 0);
    }
*/

// No.1 : Steady State Condition For VSnares	
   for (i = 0; i < len; i++ ) {      // For each Edge  
       for (j = 0; j < snareLength; j++) {       // for each molecule               
           if(edgeBag[i].vSnare & (1 << j)) {     // Present molecules   
                vali = edgeBag[i].ith;   // store the source node
                valj = edgeBag[i].jth;   // Store the target node
                // If there is a back edge from taget to source we are done.
                if (((graph[valj][vali] >= 1) && (Vedge[valj][vali] & (1 << j) )) || ((graph[valj][vali] == 2) && (Vedge2[valj][vali] & (1 << j)  )) )   {
                      C1 = C1 && 1;
                }
                // Else continue checking for the cycle
                else { 
         		// g0 is unsigned int checks if there is an edge btw two nodes
                //  It should be on some cycle, So assume that it'll be between 0 and N-2
                //  As we are Only considering elementary cycles.
                unsigned int big;
                __CPROVER_assume( big >= 1 && big <= (N - 2));
                 unsigned int path[big];   // An array to store the path taken by molecule.
             
               //  Make sure every int is between 0 and N-1 that represent the node
                for (l = 0; l < big; l++) {           // Dynamic
                      path[l] = zeroTon(N - 1);
                } 
               
	            g0  = graph[valj][path[0]];    // g0 is unsigned int checks if there is an edge btw two nodes
	            v0  = Vedge[valj][path[0]];    // snareVector gets the edgeweight of the corresponding edge.
                v2  = Vedge2[valj][path[0]];
               
                gl  = graph[path[big - 1]][vali];
	            vl  = Vedge[path[big - 1]][vali];    // snareVector gets the edgeweight of the corresponding edge.
                vl2 = Vedge2[path[big - 1]][vali];

               if ( ( (( g0 == 1) && (v0 & (1 << j))) ||  ( (g0 == 2) &&  ( (v0 & (1 << j)) || ( v2 & (1 << j)) ) )) &&  ((( gl == 1) && (vl & (1 << j))) ||  ( (gl == 2) &&  ( (vl & ( 1 << j)) || ( vl2 & (1 << j)) ) )))  {                  
                   C1 = C1 && 1;
               }

               else {
                   C1 = 0;
               }
           
           
           if ( big > 1 ) {
               for (k = 0; k < big - 1 ; k++)  {                  // Dynamic 				    	 
		           ng  = graph[path[k]][path[k+1]];
		           nv  = Vedge[path[k]][path[k+1]];
		           nv2 = Vedge2[path[k]][path[k+1]];	
                   if ( ((ng == 1) && (nv & (1 << j))) ||  ( (ng == 2) && ((nv & (1 << j)) || (nv2 & (1 << j)))) ) {
                           C1 = C1 && 1;
                   }
                   else {
                           C1 = 0;
                   }
               }
           }


           }  // else Outside closed
        }  // If closed
      }  // jth for closed    
    }   //  ith for closed 
  
  
  
// No.2 : Steady State Condition For VSnares	
   for (i = 0; i < len; i++ ) {      // For each Edge  
       for (j = 0; j < snareLength; j++) {       // for each molecule               
           if(edgeBag[i].tSnare & (1 << j)) {     // Present molecules   
                vali = edgeBag[i].ith;   // store the source node
                valj = edgeBag[i].jth;   // Store the target node
                
                if (((graph[valj][vali] >= 1) && (Tedge[valj][vali] & (1 << j) ))  || ((graph[valj][vali] == 2) && (Tedge2[valj][vali] & (1 << j) )))  {
                      C1 = C1 && 1;
                 }

                else { 
         		// g0 is unsigned int checks if there is an edge btw two nodes
                //  It should be on some cycle, So assume that it'll be between 0 and N-2
                //  As we are Only considering elementary cycles.
                unsigned int big;
                __CPROVER_assume( big >= 1 && big <= (N - 2));
     
                 unsigned int path[big];   // An array to store the path taken by molecule.
             
               //  Make sure every int is between 0 and N-1 that repres100ggent the node
                for (l = 0; l < big; l++) {           // Dynamic
                      path[l] = zeroTon(N - 1);
                } 
               
    	        g0  = graph[valj][path[0]];    // g0 is unsigned int checks if there is an edge btw two nodes
	            v0  = Tedge[valj][path[0]];    // snareVector gets the edgeweight of the corresponding edge.
                v2  = Tedge2[valj][path[0]];
               
                gl  = graph[path[big - 1]][vali];
	            vl  = Tedge[path[big - 1]][vali];    // snareVector gets the edgeweight of the corresponding edge.
                vl2 = Tedge2[path[big - 1]][vali];

               if ( ((( g0 == 1) && (v0 & (1 << j))) ||  ( (g0 == 2) &&  ( (v0 & ( 1 << j)) || ( v2 & (1 << j)) ) )) &&  ((( gl == 1) && (vl & (1 << j))) ||  ( (gl == 2) &&  ( (vl & ( 1 << j)) || ( vl2 & (1 << j)) ) )))  {                  
                   C1 = C1 && 1;
               }

               else {
                   C1 = 0;
               }
          if ( big > 1) {
               for (k = 0; k < big - 1 ; k++)  {                  // Dynamic 				    	 
		           ng  = graph[path[k]][path[k+1]];
		           nv  = Tedge[path[k]][path[k+1]];
		           nv2 = Tedge2[path[k]][path[k+1]];	
                   if (((ng == 1) && (nv & (1 << j))) ||  ( (ng == 2) && ((nv & (1 << j)) || (nv2 & (1 << j))) )) {
                           C1 = C1 && 1;
                   }
                   else {
                           C1 = 0;
                   }
               }
          }
           }  // else Outside closed
        } 
      }  // jth for closed    
    }   



    for  (i = 0; i < len; i++) {
        edgeBag[i].combinedMask = 0b0; 
        edgeBag[i].combinedMask2 = 0b0; 
        
	    total = 0b0;
        edgeBag[i].count = 0;
        edgeBag[i].count2 = 0;
        
        Ck = 0;
        Cl = 0;
        
        v = edgeBag[i].vSnare;
        t = edgeBag[i].tSnare;
        
        valj = edgeBag[i].jth;
        vali = edgeBag[i].ith;
        
        for  (j = 0; j < snareLength; j++) {   
           f = qrfusionMatrix[j];
           h = rqfusionMatrix[j];   
       
           bvv = ((Vnodes[valj] << M) | Tnodes[valj]);

           if ( (v & (1 << j))) {    
            if ((t & f) == 0) {
			  edgeBag[i].combinedMask = edgeBag[i].combinedMask | f;
              edgeBag[i].count = edgeBag[i].count + 1; 
              placeHolder = (Tnodes[valj] & f);
	          for ( l = 0; l < snareLength; l++) {
	              if  ( placeHolder & (1 << l)) { 
		               vff  =  nodeInhib[l];  // Node Inhibition of lth Rsnare   
                       if ((vff  & (b1 << bvv)) != b0) {
                            Ck = 1; 
                       }
                  }
              }
	        }
          }

         // R SNARE TIME : 

         if ( (t & (1 << j)) ) {        
             if ((v & h) == 0) {
				edgeBag[i].combinedMask2 = edgeBag[i].combinedMask2 | h;
                edgeBag[i].count2 = edgeBag[i].count2 + 1;    
                placeHolder = (Vnodes[valj] & h);
	            for ( l = 0; l < snareLength; l++) {
	                if  ( placeHolder & (1 << l)) { 
		               vff  =  nodeInhib[l + M];    // Node Inhibition of lth Qsnare
                       if ((vff  & (b1 << bvv)) != b0) {
                           Cl = 1; 
                      }
                 }
             }
	     }
    }
}    
         if(Ck == 1 || Cl == 1) {
             C2 = C2 && 1;
         }
         else {
             C2 = C2 && 0;
         }

	    for (k = 0; k < N; k++) {
	       if( k != edgeBag[i].jth) {              
               bv = ((Vnodes[k] << M) | Tnodes[k]);
	           
	           for (m = 0; m < snareLength; m++) {   	    
   
                  if (edgeBag[i].combinedMask & (1 << m)) {
		             if (Tnodes[k] & (1 << m)) {  
		                  vf = nodeInhib[m];
			              if (vf & (b1 << bv)) {  
			                  C3 = 0;
	                       }
                     }
                  }
                  
                  if (edgeBag[i].combinedMask2 & (1 << m)) {
		             if (Vnodes[k] & (1 << m)) {   
		                  vf = nodeInhib[m + M];
    			          if (vf & (b1 << bv)) {  
			                  C3 = 0;
	                       }
                      }
                  }
                }
	     }
	 }
 }  
     
    
   for  (i = 0; i < len; i++) {

        printf("The edge No.%d has this config : \n There is an edge between graph[%d][%d]" , i , edgeBag[i].ith, edgeBag[i].jth);

        printf (" vSnare =  %d \n tSnare = %d\n combinedMask = %d \n  combinedMask = %d \n counts = %d \n counts2 = %d " ,edgeBag[i].vSnare , edgeBag[i].tSnare, edgeBag[i].combinedMask, edgeBag[i].combinedMask2, edgeBag[i].count,edgeBag[i].count2);
   
   }
   
    for  (i = 0; i < snareLength; i++) {
        printf( "\n The qrfusionMatrix[[%d] = %d ", i , qrfusionMatrix[i]);
    }

    for  (i = 0; i < snareLength; i++){
        printf(" \n The rqfusionMatrix[%d] = %d ", i, rqfusionMatrix[i]);
    }

    for  (i = 0; i < N; i++){
        printf("T-Nodes[%d] = %d" , i , Tnodes[i]);
    }
    for  (i = 0; i < N; i++){
        printf("V-Nodes[%d] = %d" , i , Vnodes[i]);
    }

   for  (i = 0; i < dLen; i++){
	    printf(" edegeInhib[%d] = %d" , i ,  edegeInhib[i]);
        printf("nodeInhib[%d] = %d" , i ,  nodeInhib[i]);
  }
 

    for(i = 0;i < N ; i++) {
        for( j = 0;j < N; j++) {
            printf("Graph[%d][%d] = %d",i,j,graph[i][j]);
        }
    }

    printf("\nThe value of : \n  C1 : %d \n C2 : %d , C3 : %d C4 :%d  C5 : %d",C1,C2,C3,C4,C5);
    printf(" the value of mr.Ticks is %d and len was %d ", ticks , len);
   //assert(0);
  __CPROVER_assert( !(C1 && C2 && C3 && C4)  , "Graph that satisfy friendZoned model exists");  
 
}




