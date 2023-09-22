.PHONY: clean

all:
	docker buildx build . --platform linux/amd64,linux/arm64 --output 'type=local,dest=dist'

clean:
	rm -rf dist
