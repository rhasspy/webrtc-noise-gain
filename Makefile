.PHONY: clean armv6 armv7

all:
	docker buildx build . --platform linux/amd64,linux/arm64 --output 'type=local,dest=dist'

armv6:
	mkdir -p dist/linux_armv6
	docker buildx build . -f Dockerfile.armv6 --platform linux/arm/v6 --output 'type=local,dest=dist/linux_armv6'

armv7:
	mkdir -p dist/linux_armv7
	docker buildx build . -f Dockerfile.armv7 --platform linux/arm/v7 --output 'type=local,dest=dist/linux_armv7'

clean:
	rm -rf dist
