import sys
import time

import numpy as np

from evaluation import *
from movieLensParser import *

def LearnBaseHypothesis(Users, Ratings):
    """ Compute for each user the average of her ratings """
    P = {} # Dictionary from user to ratings
    for rate in Ratings:
        if rate.idUser not in P:
            P[rate.idUser] = []
        P[rate.idUser].append(int(rate.rating))

    H = {}
    for p in P:
        H[p] = np.mean(P[p])
    return H


def BiasFromMean(Users, Ratings, Items):
    """ Compute the prediction based on <bias from mean> [ref] """
    P1 = LearnBaseHypothesis(Users, Ratings)
    P2 = {}
    for p in P1:
        P2[p] = {}

    for Item in Items:
        SiX = filter(lambda z: z.idItem == Item.id, Ratings)
        n = len(SiX)

        bias = 0.0	
        if n > 0:
            for r in SiX:
                bias += float(r.rating) - P1[r.idUser] 
            bias = bias/n

        for p in P1:
            P2[p][Item.id] = round(P1[p] + bias)

    return P2
 

def SlopeOne(Users, Items, Ratings):
    """ Compute the average deviation matrix for SlopeOne """
    # Set of rating per user
    C = {}
    for u in Users:
        C[u.id] = []
    for r in Ratings:
        C[r.idUser].append(r)

    start = time.time()

    # Compute the set for S_ij(X) (see the SlopeOne paper)
    SX = {}
    for i in Items:
        SX[i.id] = {}
        
    for u in C:
        for j,rj in enumerate(C[u]):
            for ri in C[u][j+1:]:
                if ri.idItem not in SX[rj.idItem]:
                    SX[rj.idItem][ri.idItem] = (0.0, 0)
                a, b = SX[rj.idItem][ri.idItem]
                SX[rj.idItem][ri.idItem] = (a+(rj.rating - ri.rating), b+1)

                if rj.idItem not in SX[ri.idItem]:
                    SX[ri.idItem][rj.idItem] = (0.0, 0)
                a, b = SX[ri.idItem][rj.idItem]
                SX[ri.idItem][rj.idItem] = (a+(ri.rating - rj.rating), b+1)

    print("Computed SX:", time.time() - start)
    start = time.time()

    # Compute the average matrix  
    Dev = {}              
    for i in SX:
        for j in SX[i]:
            a, b = SX[i][j]
            if b > 0:
                if i not in Dev:
                    Dev[i] = {}
                Dev[i][j] = float(a)/b
    print("Computed M:", time.time() - start)
    start = time.time()

    # Compute the relevant items for each pair (user, item) not yet rated
    I = {}
    for i in Items:
        I[i.id] = i

    R = {}
    for u in Users:
        R[u.id] = {}
        rated_items = list(map(lambda z: z.idItem, C[u.id]))
        
        for j in Dev:
            R[u.id][j] = []
            for i in rated_items:
                if i != j and i in Dev[j]:
                    R[u.id][j].append(i)

    print("Computed R:", time.time() - start)
    start = time.time()

    # Compute the bias on the average for slope one
    PS1 = {}
    for u in map(lambda z: z.id, Users):
        if C[u]:
            rated_items = set(map(lambda z: z.idItem, C[u]))
            u_bar = np.mean(list(map(lambda z: float(z.rating), C[u])))
            PS1[u] = {}
            for j in Items:
                PS1[u][j.id] = u_bar
                if j.id not in rated_items and j.id in R[u]:
                    PS1[u][j.id] = u_bar
                    card = float(len(R[u][j.id]))
                    for i in R[u][j.id]:
                        PS1[u][j.id] += 1.0/card*Dev[j.id][i]

    print("Computed PS1:", time.time() - start)

    return PS1



#------------------------------------------
#              MAIN ENTRY POINT
#------------------------------------------
if __name__ == "__main__":
    args = parser.parse_args()

    print("Filenames: ", args.users, args.items, args.trainset)
    if args.users and args.items and args.trainset:
        Users = ParseUsers(args.users)
        Items = ParseItems(args.items)
        TrainingSet = ParseRatings(args.trainset)

        print("Users:", len(Users))
        print("Items:", len(Items))
        print("Training Set:", len(TrainingSet))
    else:
        print("Please, fill in all the input parameters")

    start = time.time()
    
    P1 = SlopeOne(Users, Items, TrainingSet)
    
    end = time.time()
    print("Elapsed time: ", end - start)

    Y = []
    X1 = []
    TestSet = ParseRatings(args.validateset)
    for r in TestSet:
        if r.idUser in P1 and r.idItem in P1[r.idUser]:
            X1.append(round(P1[r.idUser][r.idItem]))
            Y.append(int(r.rating))
        else:
            print(r)
            print(P1)
            sys.exit(0)
    

    print("SlopeOne: ", Evaluate(X1, Y))

    end = time.time()
    print("Elapsed time: ", end - start)

    sys.exit(0)

	# Rating as real numbers (averages)
    P1 = LearnBaseHypothesis(Users, TrainingSet)

	# Rating as int numbers 
    P2 = {}
    for p in P1:
        P2[p] = round(P1[p])

    if False:
        P3 = BiasFromMean(Users, TrainingSet, Items)
        # TEST PHASE
        TestSet = ParseRatings(args.validateset)
        Y = []
        X1 = []
        X2 = []
        X3 = []
        for r in TestSet:
            X1.append(P1[r.idUser])
            X2.append(P2[r.idUser])
            X3.append(P3[r.idUser][r.idItem])
            Y.append(int(r.rating))
        print("PER USER AVERAGE - rating as real: ", Evaluate(X1, Y))
        print("PER USER AVERAGE - rating as int:  ", Evaluate(X2, Y))
        print("BIAS FROM MEAN   - rating as int:  ", Evaluate(X3, Y))
