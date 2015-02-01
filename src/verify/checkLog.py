import io
import math
import argparse

def checkTimeStamp(currTime, prevTime, rate):
	if int(currTime) == -1:
		return -1
	temp = (float(currTime) - float(prevTime))
	#print temp

	return temp == 0

def getTimeStamp(line):
	str_array = line.split(",")
	if len(str_array) < 16:
		return -1
	return str_array[6]  

def file_len(filename):
	return sum(1 for line in open(filename))

def readDataFile(filename, rate=0.02):
	lines = file_len(filename)
	log = open(filename, 'r')

	prevLine = ""
	currLine = ""
	isDiff = False
	count = 0
	for line in log:
		if count == 0:
			prevLine = line
			count += 1
			continue
		currLine = line
		#print line

		isDiff = checkTimeStamp(getTimeStamp(currLine), getTimeStamp(prevLine), rate)
		if isDiff == -1:
			break
		if not isDiff:
			print count

		prevLine = currLine
		count += 1


	log.close()

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("file", help="Name of file to be parsed")
	parser.add_argument("-r", "--rate", help = "Refresh rate of sensor")

	args = parser.parse_args()

	readDataFile(args.file, args.rate)

if __name__ == '__main__':
	main()





