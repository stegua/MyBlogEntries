from math import sqrt

# ---------------------- EVALUTION FUNCTIONS -------------------	
def MAE(X, Y):
	""" Mean Absolute Error (MAE) """
	r = 0.0
	for x,y in zip(X,Y):	
		r += abs(x-y)
	return r/len(X)

def RMSE(X, Y):
	""" Root Mean Square Error (RMSE) """
	r = 0.0
	for x,y in zip(X,Y):	
		r += (x-y)*(x-y)
	return sqrt(r/len(X))

def NMAE(X, Y):
	""" Normalized Mean Absolute Error (NMAE) """
	mae = MAE(X,Y)
	r_min = min(Y)
	r_max = max(Y)
	r = mae/(r_max-r_min)
	return r

def Evaluate(X, Y):
    """ Compute MAE, RMSE, NMAE """
    return map(lambda x: round(x, 4), [MAE(X,Y), NMAE(X,Y), RMSE(X,Y)])
