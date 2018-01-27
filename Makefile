all: libsai
libsai: 
	docker run -it -v${PWD}/mlnx_sai:/make mlnx-sai-build:0.0.3 make -C /make