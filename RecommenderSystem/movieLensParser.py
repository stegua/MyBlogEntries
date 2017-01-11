import sys
import operator
from collections import namedtuple

# u.item     -- Information about the items (movies); this is a tab separated
#   list of
#   movie id | movie title | release date | video release date |
#   IMDb URL | unknown | Action | Adventure | Animation |
#   Children's | Comedy | Crime | Documentary | Drama | Fantasy |
#   Film-Noir | Horror | Musical | Mystery | Romance | Sci-Fi |
#   Thriller | War | Western |
#
#   The last 19 fields are the genres, a 1 indicates the movie
#    is of that genre, a 0 indicates it is not; movies can be in
#   several genres at once.
#   The movie ids are the ones used in the u.data data set.
Item = namedtuple("Item", ['id', 'title', 'releaseDate', 'videoDate', 'link', \
		'unknown', 'Action', 'Adventure', 'Animation', 'Children', 'Comedy', 'Crime', 'Documentary', \
		'Drama', 'Fantasy', 'FilmNoir', 'Horror', 'Musical', 'Mystery', 'Romance', \
		'SciFi', 'Thriller', 'War', 'Western'])


# u.user     -- Demographic information about the users; this is a tab
#   separated list of
#   user id | age | gender | occupation | zip code
#   The user ids are the ones used in the u.data data set.
User = namedtuple("User", ['id', 'age', 'gender', 'occupation', 'zip'])


# u.data     -- The full u data set, 100000 ratings by 943 users on 1682 items.
#   Each user has rated at least 20 movies.  Users and items are
#   numbered consecutively from 1.  The data is randomly
#   ordered. This is a tab separated list of 
#   user id | item id | rating | timestamp. 
#   The time stamps are unix seconds since 1/1/1970 UTC
Rating = namedtuple("Rating", ['idUser', 'idItem', 'rating', 'timestap'])


# List of occupation
Occupations = ['administrator', 'artist', 'doctor', 'educator', 'engineer', 'entertainment', 'executive', 'healthcare', 'homemaker', \
	      'lawyer', 'librarian', 'marketing', 'none', 'other', 'programmer', 'retired', 'salesman', 'scientist', 'student', 'technician', 'writer']


# Parse data
def ParseItems(filename):
	Ls = []
	data = open(filename)
	for line in data:
		row = line.replace('\n','').split('|')
		row[0] = int(row[0])
		tmp = Item._make(row)
		Ls.append(tmp)
	return Ls

def ParseUsers(filename):
	Ls = []
	data = open(filename)
	for line in data:
		row = line.replace('\n','').split('|')
		row[0] = int(row[0])
		tmp = User._make(row)
		Ls.append(tmp)
	return Ls

def ParseRatings(filename):
	Ls = []
	data = open(filename)
	for line in data:
		row = line.replace('\n','').split('\t')
		row[0] = int(row[0])
		row[1] = int(row[1])
		row[2] = int(row[2])
		tmp = Rating._make(row)
		Ls.append(tmp)
	return Ls

# Command line arguments
import argparse
    
parser = argparse.ArgumentParser()
parser.add_argument('--version', action='version', version='0.0.1')
parser.add_argument('-u', '--users', help='users data filename', default='./data/ml-100k/u.user')
parser.add_argument('-i', '--items', help='items data filename', default='./data/ml-100k/u.item')
parser.add_argument('-t', '--trainset', help='training set filename', default='./data/ml-100k/u1.base')
parser.add_argument('-v', '--validateset', help='validation set filename', default='./data/ml-100k/u1.test')

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
