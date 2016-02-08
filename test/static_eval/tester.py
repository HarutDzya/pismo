#!/usr/bin/python2.7

from __future__ import print_function
import sys, os

if len(sys.argv) != 3:
	print("Usage: " + sys.argv[0] + " input_file threshold_value")
	exit(1)

threshold = float(sys.argv[2])
headerTemplate = "{:^12}|{:^12}|{:^12}|{:^12}|{:^12}\n" 
output = [headerTemplate.format("pismo", "stockfish", "gull", "status", "FEN")]
template = "{:^12.2f}|{:^12.2f}|{:^12.2f}|{:^12}|{:^12}\n" 
output.append("-" * 70 + "\n")
testCount = 0
greatTestCount = 0
goodTestCount = 0
badTestCount = 0
for fen in open(sys.argv[1]):
	argument = '"position fen ' + fen.rstrip() + '"'
	stockfishScore = float(os.popen("/bin/bash -c './stockfish/stockfish <<< " + argument + "'").read())
	gullScore = float(os.popen("/bin/bash -c './gull/gull <<< " + argument + "'").read())
	pismoScore = float(os.popen("/bin/bash -c './pismo/pismo <<< " + argument + "'").read())
	if stockfishScore <= pismoScore <= gullScore or gullScore <= pismoScore <= stockfishScore:
		status = "great"
		greatTestCount += 1
	elif pismoScore < min(stockfishScore, gullScore) and pismoScore > min(stockfishScore, gullScore) - threshold:
		status = "good"
		goodTestCount += 1
	elif pismoScore > max(stockfishScore, gullScore) and pismoScore < max(stockfishScore, gullScore) + threshold:
		status = "good"
		goodTestCount += 1
	else:
		status = "bad"
		badTestCount += 1
	output.append(template.format(pismoScore, stockfishScore, gullScore, status, fen.rstrip()))
	testCount += 1

print("Total: {} (threshold {:.2f})".format(testCount, threshold))
print("great - {:.2f}% ({} positions)".format(greatTestCount * 100.0 / testCount, greatTestCount))
print("good - {:.2f}% ({} positions)".format(goodTestCount * 100.0 / testCount, goodTestCount))
print("bad - {:.2f}% ({} positions)".format(badTestCount * 100.0 / testCount, badTestCount), end = "\n\n")
print(*output)
