import random

read_file = open("nfl_teams.txt", 'r')
write_file = open("rand_teams.txt", 'w')

teams = read_file.readlines()
num = len(teams)
for x in range(0, 1000):
	team = random.randrange(0, num)
	write_file.write(teams[team].strip()+'\n')

read_file.close()
write_file.close()
