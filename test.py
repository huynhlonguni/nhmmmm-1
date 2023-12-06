import os, subprocess, time, sys

dirp = sys.argv[1]
prog = os.path.join(dirp, "main.exe")

counter = 0
correct = 0

for file in os.listdir(dirp):
	if file.endswith(".inp"):
		path = os.path.join(dirp, file)
		print(f"Test {file}")
		timeStarted = time.time()
		process = subprocess.run([prog, path, "test.out"], capture_output = True, text = True)
		timeDelta = time.time() - timeStarted
		output = process.stdout.strip()
		print(f"Time: {timeDelta}")
		print(f"Output: {output}")
		if (os.path.isfile(path.replace(".inp", ".out"))):
			counter += 1
			with open(path.replace(".inp", ".out")) as f:
				expected = f.read().strip()
				print(f"Expect: {expected}")
			if output.splitlines() == expected.splitlines():
				correct += 1
				print("Output matched!")
		print("")

print(f"Passed {correct}/{counter} tests")