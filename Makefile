.PHONY: all
all: daisy firmware

.PHONY: daisy
daisy:
	${MAKE} -C 3rd_party/libDaisy -j ${shell nproc}

.PHONY: firmware
firmware:
	${MAKE} -C src/Template -j ${shell nproc}

.PHONY: clean
clean:
	rm -rf 3rd_party/libDaisy/build
	rm -rf src/Template/build
