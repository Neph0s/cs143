import os
import pdb

my_output = os.popen("make dotest").read()
std_output = os.popen("lexer test.cl").read()

begin_index = my_output.index("#name")
my_output = my_output[begin_index:]

while 1:
	try:
		my_end = my_output.index("\n")
		std_end = std_output.index("\n")
	except:
		print('End of file')
		break

	if my_output[0: my_end] != std_output[0:std_end]:
		print("Wrong!!!!!!!")
		print("My output:{0} \n Standard output: {1} \n".format(my_output[0: my_end], std_output[0: std_end]))
		#pdb.set_trace()

	my_output = my_output[my_end+1: ]
	std_output = std_output[std_end+1: ]