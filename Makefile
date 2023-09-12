.PHONY: python run run-python docker clean

all:
	g++ -o test test.cpp -Iinstall/include -Iinstall/include/webrtc-audio-processing-1 -Iinstall/include/webrtc-audio-coding-1 -Linstall/lib/x86_64-linux-gnu -lwebrtc-audio-processing-1 -lwebrtc-audio-coding-1 -pthread -lrt -DWEBRTC_LIBRARY_IMPL -DWEBRTC_POSIX

python:
	python3 setup.py install

python-test:
	LD_LIBRARY_PATH=${PWD}/install/lib/x86_64-linux-gnu python3 test.py

run:
	LD_LIBRARY_PATH=${PWD}/install/lib/x86_64-linux-gnu ./test

docker:
	docker buildx build . --platform linux/amd64,linux/arm64 --output 'type=local,dest=dist'

clean:
	rm -rf dist
