MODULE_NAME = intel_gpu.o

USER_CFLAGS = -DDEBUG -Ilil/src

all: lil
	make module

lil:
	git clone https://github.com/Matt8898/lil.git lil

include /opt/foxos_sdk/module.mak