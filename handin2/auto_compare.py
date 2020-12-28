import os
import pdb
import subprocess
import argparse

parser = argparse.ArgumentParser(description = 'input good or bad')
parser.add_argument('-m','--mode',default = 'good', choices = ['good', 'bad', 'good2', 'bad2', 'good3'])
args = parser.parse_args()

file_name = args.mode + '.cl'

test_mine_ins = './myparser ' + file_name
test_std_ins = './lexer ' + file_name + ' | ./../../bin/parser ' + file_name

my_status, my_output = subprocess.getstatusoutput(test_mine_ins)
std_status, std_output = subprocess.getstatusoutput(test_std_ins)
pdb.set_trace()
hist_std_output = '' 
sign = 1
while 1:
	try:
		my_end = my_output.index("\n")
	except:
		sign += 1

	try:
		std_end = std_output.index("\n")
	except:
		sign += 1

	if (sign == 3):
		print('Success, reach EOF')
		break
	elif (sign == 2):
		print('Fail, number of lines do not match')
		break

	if my_output[0: my_end] != std_output[0:std_end]:
		
		print("Wrong!!!!!!!")
		print("My output:{0} \nStandard output: {1} \n".format(my_output[0: my_end], std_output[0: std_end]))

	hist_std_output += std_output[: std_end+1]
	my_output = my_output[my_end+1: ]
	std_output = std_output[std_end+1: ]