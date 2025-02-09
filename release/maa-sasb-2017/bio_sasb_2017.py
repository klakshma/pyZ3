#!/usr/bin/python

##############################################################################
# Author: Ashutosh gupta and ankit shukla 

# # This version is for the submissted paper at SASB-2017 ###
# It only solves necc. condition of VTS. ##########
# For sufficient condition please refer main repo bio.py #####

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##############################################################################

from z3 import *
import argparse
import time

#----------------------------------------------------
# input parsing
# input number of nodes and molecules
parser = argparse.ArgumentParser(description='Auto testing for TARA')
parser.add_argument("-M","--mols", type=int, default=4, help = "number of molecules")
parser.add_argument("-N","--nodes", type=int, default=2, help = "number of nodes")
parser.add_argument("-Q","--pedges", type=int, default=1, help = "max no.of parallel edges btw two nodes")
parser.add_argument("-V","--variation", type=int, default=1, help = "model of the biological system")
#parser.add_argument("-C","--connected", type=int, default=3, help = "graph connectivity want to check")
parser.add_argument("-E","--edges", type=int, default=3, help = "graph connectivity want to check")

args = parser.parse_args()

M = args.mols
N = args.nodes
Q = args.pedges
V = args.variation
#C = args.connected

bag = []
bag.append ('1. Boolean function on nodes and Boolean function on the edges.') 
bag.append ('2. Only Boolean function on the edges.') 
bag.append ('3. Boolean function on nodes and SNARE-SNARE inhibition on edges.')
bag.append ('4. Only SNARE-SNARE inhibition on edges.')
bag.append ('5. Only Boolean function on nodes.')
bag.append ('6. No regulation at all.')

for i in range(6):
    if V == i + 1:
        print ' '
        print 'Default/choosen vesicular traffic system is regulated by ...',
        print bag[i]
        print ' '
        print 'Use -V option to change the regulated variation.' 
        for i in bag:
            print i 
        print ' '
        break

#----------------------------------------------------
# Constraint generation 
edge = [ [ [ Bool ("e_{}_{}_{}".format(i,j,q)) for q in range(Q)] for j in range(N)] for i in range(N)]

dump = [ [ [Bool ("d_{}_{}_{}".format(i,j,q)) for q in range(Q)] for j in range(N)] for i in range(N)]

r1 = [ [Bool ("r1_{}_{}".format(i,j)) for j in range(N)] for i in range(N)]

node = [ [Bool ("n_{}_{}".format(i,k)) for k in range(M)] for i in range(N)]

active_node = [ [Bool ("a_{}_{}".format(i,k)) for k in range(M)] for i in range(N)]

presence_edge = [ [ [ [Bool ("e_{}_{}_{}_{}".format(i,j,q,k)) for k in range(M)] for q in range(Q)] for j in range(N)] for i in range (N)]

active_edge = [ [ [ [Bool ("b_{}_{}_{}_{}".format(i,j,q,k)) for k in range(M)] for q in range(Q)] for j in range(N)] for i in range (N)]

r = [ [ [ [Bool ("r_{}_{}_{}_{}".format(i,j,k,z)) for z in range(N-1)] for k in range(M)] for j in range(N)] for i in range (N)]

p = [ [Bool ("p_{}_{}".format(k,k1)) for k1 in range(M)] for k in range(M)]

sorts = [BoolSort() for m in range( M )]

f_n = [Function ("an_{}".format(m), *sorts) for m in range(M)] 

f_e = [Function ("ae_{}".format(m), *sorts) for m in range(M)] 

st = time.time()

#    
# Function: no regulation on the node.
# The present molecules at nodes are all active.
def f_nn():
    A_list = []
    for i in range(N):
        for k in range(M):
            lhs = node[i][k] == active_node[i][k]
            A_list.append(lhs)
    return And(A_list)

# No regulation on edge.
# The present molecules on the edge are all active.
def f_ne():
    A_list = []
    for i in range(N):
        for j in range(N):
            if i == j:
                continue
            for q in range(Q):
                for k in range(M):
                    lhs = presence_edge[i][j][q][k] == active_edge[i][j][q][k]
                    A_list.append(lhs)
    return And(A_list)

# Activity of a moolecule k on a node/edge is  
# defined as a Boolean function of presence of 
# Other molecule present on that node/edge.
# A0. active_node[k] =  f_n[k](\/_{k1 != k} node(k1)) 
def f_bn():
   # print "I am here!!"
    s = []
    A_list = []
    for k in range(M):
        f = f_n[k]
        for i in range(N):
            del s[:]
          #  for t in QVars:
          #      s.append( t )
            for k1 in range(M):
                if k1 == k:
                    continue
                s.append( node[i][k1])
            f_app = f(s)
            l = Implies( node[i][k], active_node[i][k] == f_app )
            A_list.append(l)
    return And( A_list )


# Activity of the molecules on the edge is driven by the 
# chosen boolean function. 
# A1. active_edge[k] = f_e[k](\/_{k1 != k} presence_edge(k1))
def f_be():
    s = []
    A_list = []
    for i in range(N):
        for j in range(N):
            if i == j:
                continue
            for q in range(Q):
                for k in range(M):
                    del s[:]
                    f = f_e[k]
                   # for t in QVars:
                   #     s.append( t )
                    for k1 in range(M):
                        if k1 == k:
                            continue
                        s.append(presence_edge[i][j][q][k1])
                    l  =  Implies(presence_edge[i][j][q][k], active_edge[i][j][q][k] == f(s)) 
                    A_list.append(l)
    return And( A_list )

# Inhibition of the edges are driven by the pairing matrix.
# If there is a molecule k present on the edge(i,j) and with non-zero pairing matrix
# the evry pairing matrix elements are present on that node makes that molecule inactive.
# else its active.
# { e(i,j,k) and (exists_k': p(k,k') and [p(k,k') -> e(i,j,k')] )) } -> ~ b(i,j,k)   
# { e(i,j,k) and ~(exists_k': p(k,k') and [p(k,k') -> e(i,j,k')] )) } -> b(i,j,k)   

def f_se():
    A_list = []
    for i in range(N):
        for j in range(N):
            if i == j:
                continue
            for q in range(Q):
                for k in range(M):
                    lhs = presence_edge[i][j][q][k]
                    l1 = False
                    l2 = True
                    for k1 in range(M):
                        if k1 == k:
                            continue
                        l1 = Or( p[k][k1], l1)
                        l2 = And( Implies( p[k][k1], presence_edge[i][j][q][k1]) , l2)   
                    l = Implies ( And( lhs, And( l1,l2)), Not( active_edge[i][j][q][k]) )
                    ll = Implies ( And ( lhs, Not (And ( l1,l2)) ), active_edge[i][j][q][k]) 
                    A_list.append(And ( l, ll) )
    return And (A_list)

A0 = True 
A1 = True

if V == 1:
    A0 = f_bn() 
    A1 = f_be()
    #print A0
    #print A1
    #exit(0)
elif V == 2:
    A0 = f_nn()
    A1 = f_be()
    #print A0
    #print A1
elif V == 3:
    A0 = f_bn()
    A1 = f_se()
    #print A1
    #exit(0)
elif V == 4:
    A0 = f_nn()
    A1 = f_se()
elif V == 5:
    A0 = f_bn()
    A1 = f_ne()
else:
    A0 = f_nn()
    A1 = f_ne()

xx = time.time() - st
print "Building A's took...", str(xx)

st = time.time()
# print A0
# print A1

#1. label(E) subset label(N)
# e_ijk -> aik
# e_ijk -> ajk

# -- Updated.
# e_ijk -> a_ik and a_jk
C1 = True
for i in range(N):
    for j in range(N):
        if i == j :
            continue
        for q in range(Q):
            for k in range(M):
                C1 = And (Implies( presence_edge[i][j][q][k], And( node[i][k], node[j][k]) ),C1)

#2. Self edges not allowed. 
# not e_ii  
C2 = True
for i in range(N):
    for q in range(Q):
        C2 = And( Not(edge[i][i][q]), C2)


#3. Multiple(parallel) edges are allowed between two nodes. 
# But we restrict it to two.

#-- Update:
# Implemented explicitely using another dimension in edge [:Q].
#C3 = True

#4. Condition on p_kk' : 
# \/_{x<M/2,y>=M/2} !p(x,y) and !p(x,y)

C4 = True
for x in range(M):
    for y in range(M):
        if (((x < M/2) and (y < M/2)) or ((x>=M/2) and (y >=M/2))):
            C4 = And( Not(p[x][y]),C4 )


#5. Activity on the node.
C5 = True 
A1_list = []
for i in range(N):
    for k in range(M):
        l = Implies ( active_node[i][k], node[i][k])
        A1_list.append(l) 
C5 = And (A1_list)
#print C5

c = time.time() - st
print "Building C's took...", str(c)
# ----------------------------------------------------------------
st = time.time()

# MAIN Constraints:

# F_0
# \/_k e_ijk -> e_ij  at evry edge there is an active molecules which is present.
# Involve equality and we'll might  it while doing validity check.
F0 = True
for i in range(N):
    for j in range(N):
        if j == i:
            continue
        for q in range(Q):
            rhs = False
            for k in range(M):
                # Change the presence to activity from the last change on 1st June.
                rhs = Or( presence_edge[i][j][q][k], rhs )
            F0 = And (Implies(rhs, edge[i][j][q]),F0)
#print F0

# F1:  b_ijk -> e_ijk   
# If molecule is active on an edge then it should be present on the edge. \newline
F1 = True
F1_list = []
for i in range(N):
    for j in range(N):
        if j == i:
            continue
        for q in range(Q):
            for k in range(M):
                F1_list.append( Implies(active_edge[i][j][q][k], presence_edge[i][j][q][k]) )
F1 = And( F1_list )
#print F1

A_list = []
for i in range(N):
    for j in range(N):
	if i == j:
	    continue
        for k in range(M):
            lhs = False
            rhs = False
            for q in range(Q):
                rhs = Or( presence_edge[i][j][q][k], rhs)
            for z in range(N-1):
                lhs = Or( r[j][i][k][z], lhs)
            l  = Implies( rhs, lhs)
            A_list.append(l)
F3 = And(A_list)
#print F3
#exit(0)

#F2 New reachability and stability condition-----
# States reachability definition: 
# nodes i,j is reachable with kth molecule in z steps if 
# there is an edge between i'' and j (i != i'') with k present on that edge.
# and i'' is reachable from i in z steps.

#-- Update with multiple edges.
# for_all i,j,k,z : if z = 0 -> [r(i,j,k,0) -> exists q : (e(i,j,q,k)) -> ]  
#                   else: r(i,j,k,z) -> exists l,q (e(i,l,q,k) and r(l,q,k,z-1)) 

#--- Old specifiction.
# /\_{i,j,k,p, i!=j} r(i,j,k,p) -> \/_{l!=i || j}  e(i,l,k) and r(l,j,k-1)

# Have also added one length reachability with F2. 
# i,j is reachaable in one step if there is a direct edge between them.
# r_{i,j,k,1} -> e_{i,j,k} : In our case 1 is 0.
F2 = True
A_list = []
for i in range(N):
    for j in range(N):
	if i == j:
	    continue
        for k in range(M):
            for z in range(N-1):
                pe = False
                if z == 0: 
                    for q1 in range(Q):
                        pe = Or( presence_edge[i][j][q1][k], pe)
                    A_list.append( Implies(r[i][j][k][0], pe) ) 
                else:
                    lhs = False
                    for l in range(N):
                        if i == l or j == l:
                            continue
                        for q1 in range(Q):
                            pe = Or( presence_edge[i][l][q1][k], pe)
                        lhs = Or(And ( r[l][j][k][z-1],pe), lhs)   
                    w  = Implies( r[i][j][k][z],lhs)
                    A_list.append(w)
F2 = And(A_list)
#print F2


fss = time.time() - st
print "Building F0-F3's took...", str(fss)

st = time.time()
# F4: Fusion rules:
# For an edge to be valid, at least one'
# SNARE pair on the vesicle and target compartment must be active, and have a
# non-zero entry in the pairing matrix

# -- Updated specification.
# forall_{i,j,q} e(i,j,q) -> exists_{k,k'}: b(i,j,q,k) and a(j,k') and p(k,k')

# /\_{i,j} ( \/_k e_{i,j,k}) -> \/_{k,k'} (b_{i,j,k} and a'_{j,k'} and p_{k,k'}) 
# and /\_k b_{i,j,k} -> not(\/_{j' != j} (\/_k'' a'_{j',k''} and p_{k,k''}))  
F4 = True
A_list = []
for i in range(N):
    for j in range(N):
	if i == j:
	    continue
        for q in range(Q):
	    lhs = False    
            for k in range(M):
	        for k1 in range(M): 
                    if k == k1:
                        continue
	            lhs = Or (  And( active_edge[i][j][q][k], And ( active_node[j][k1],p[k][k1]) ), lhs)  
            w =  Implies(edge[i][j][q],lhs)
            A_list.append(w)
F4 = And(A_list)
#print F4

# Fusion respects the graph structure by the vesicle (edge) under consideration, 
# fusing only with its target node and not with any other node.

# --Updated specification with multiple edges.
# forall_{i,j,q,k} b(i,j,q,k) -> ~ [exits_{j',k"}: a(j',k") and p(k,k")]
F5 = True
A_list = []
for i in range(N):
    for j in range(N):
	if i == j:
	    continue
        for q in range(Q):
            for k in range(M): 
                lhs = False
	        for j1 in range(N):
                    if j == j1:
                        continue
	            for k11 in range(M):
                        if k == k11:
                            continue
		        lhs = Or(And ( active_node[j1][k11], p[k][k11]), lhs)
	        w = Implies( active_edge[i][j][q][k], Not(lhs))
                A_list.append(w)
F5 = And(A_list)

fsss = time.time() - st
print "Building F4-F5's took...", str(fsss)


st = time.time()
#----3 Connectivity --------

# Only present edges can be dropped.
# D0 = dump(i,j,q) -> e(i,j,q) 
D1_list = []
for i in range(N):
    for j in range(N):
        if i == j:
            continue
        for q in range(Q):
            D1_list.append( Implies (dump[i][j][q], edge[i][j][q]) )
D0 = And (D1_list)

# General encoding rather than one one call.
#def at_least(u):
#    for i in range(L):
#        for j in range(i+1,L):
#            D0 = Implies ( d1[i][j]  
#            
#
#def exactly_u(u):
#    at_least(u)
#    at_most(u)
#
# Summation of d_{i,j} == 2
# Flattening the array. Avoid i == j. 
d1 = []
for i in range(N):
    for j in range(N):
        if i == j:
            continue
        for q in range(Q):
            d1.append(dump[i][j][q])
L = len(d1)


#pair = []
#for i in d1:
#    pair.append((i,1))
#
##print pair
#D1 = PbEq(pair,C)
#
# At least 2
D1_list = []
for i in range(L-1):
    for j in range(i+1,L):
        D1_list.append( And( d1[i], d1[j]) )
D1 = Or(D1_list)

# At least 3
D1_list = []
for i in range(L-2):
    for j in range(i+1,L-1):
        lhs = And ( d1[i], d1[j])
        for k in range(j+1,L):
            D1_list.append( And( lhs,d1[k]))
D2 = Or(D1_list)
D2 = Not (D2)

st = time.time()
# forall_{i,j} r'(i,j) -> exists_{q} [e(i,j,q) and not d(i,j,q)] Or  exists_{i'!=i}: [r'(i',j) and [exists_{q}: e(i,i',q) and ~d(i,i',q)] ]   
# do we need length reach?
# D3 = True
A_list = []
for i in range(N):
    for j in range(N):
        if i == j:
            continue
        bhs = False
        for q in range(Q):
            bhs = Or( And( edge[i][j][q], Not(dump[i][j][q]) ), bhs) 
        rhs = False
        for l in range(N):
            if i == l or j == l:
                continue
            for q in range(Q):
                rhs = Or( And( edge[i][l][q], Not(dump[i][l][q])), rhs)
            rhs = And( rhs, r1[l][j])
        # w = Implies( r1[i][j], Or (bhs , rhs) )
        w = Implies( Or(bhs , rhs), r1[i][j] )
        A_list.append(w)
D4 = And(A_list)
#print D4
# sys.exit()


# ---- ---- --- 
# Graph becomes disconnected.
# Ensure that there is no path between some nodes i,j 
#                 in the underlying undirected graph. 
D3 = True
for i in range(N):
    for j in range(N):
        if i == j:
            continue
        rijji = Or(r1[i][j], r1[j][i])
        D3 = And( rijji, D3) 
D3 = Not(D3)
#print D4

dx = time.time() - st
print "D0-D3 Building took", str(dx)

dxx = time.time() - st
print "D0-D4 Building took", str(dxx)


fbx = dx + dxx + xx + c + fss + fsss

#Init = (presence_edge[0][1][0] == True)
#Init2 = (p[0][3] == False)
#Init3 = (p[1][2] == False)
# Create Solver and add constraints in it.

s = Solver()
s.add(A0,A1, C1,C2, C4,C5, F0,F1,F2,F3,F4,F5, D0,D1,D2,D3,D4)

print "solving...\n"
st = time.time()
print s.check()

b = time.time() - st 
print "Solving took...", str(b + fbx)



def dump_dot( filename, m ) :
    print "dumping dot file: ", filename
    dfile = open(filename, 'w')
    dfile.write("digraph prog {\n")
    for i in range(N):
        node_vec = str(i)+":"
        for k in range(M):
            if is_true(m[node[i][k]]) :
                node_vec = node_vec + "1"
            elif is_false(m[node[i][k]]):
                node_vec = node_vec + "0"
            if is_true(m[active_node[i][k]]) :
                node_vec = node_vec + "-"
        dfile.write( str(i) + "[label=\"" + node_vec + "\"]\n")
        for j in range(N):
            if i == j:
                continue
            for q in range(Q):
                for k in range(M):
                    if q == 0:
                        style = "solid"
                        if is_true( m[dump[i][j][q]] ):
                            style="dashed"
                        if is_true(m[presence_edge[i][j][q][k]]):
                            label = str(k)
                            color = "black"
                            if is_true(m[active_edge[i][j][q][k]]):
                                color = "green"
                                for k1 in range(M):
                                    if is_true(m[active_node[j][k1]]) and is_true(m[p[k][k1]]):
                                        color = "red"
                                        break
                            dfile.write( str(i) + "-> " + str(j) + "[label=" + label +",color=" + color + ",style=" + style + "]" +"\n" )
                    if q == 1:
                        style = "solid"
                        if is_true( m[dump[i][j][q]] ):
                            style="dashed"
                        if is_true(m[presence_edge[i][j][q][k]]):
                            label = str(k)
                            color = "yellow"
                            if is_true(m[active_edge[i][j][q][k]]):
                                color = "pink"
                                for k1 in range(M):
                                    if is_true(m[active_node[j][k1]]) and is_true(m[p[k][k1]]):
                                            color = "blue"
                                            break
                            dfile.write( str(i) + "-> " + str(j) + "[label=" + label +",color=" + color + ",style=" + style + "]" +"\n" )

    dfile.write("}\n")

if s.check() == sat:
    m = s.model()
    #print m
    #print "Printing the model..."
    for d in m.decls():
        print "{} = {}".format(d.name(), m[d])
#    print 
    xc = [ [ m[p[i][j]] for j in range(M) ] for i in range(M) ]
    s = [[ [ [m[active_edge[i][j][q][k]] for k in range (M)] for q in range(Q)] for j in range(N) ] for i in range(N) ]
    t = [ [ [m[edge[i][j][q]]  for q in range(Q)] for j in range(N) ] for i in range(N) ]
    print 'Printing the pairing matrix'
    print_matrix(xc)
   # print ' '
    #print t
    dump_dot( "/tmp/bio.dot", m )
else:
    print "failed to solve"
