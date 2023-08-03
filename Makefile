MODULE_NAME = intel_gpu.o

USER_CFLAGS = -DDEBUG -Ilil/include/lil -Ilil/include

all: lil
	make module

lil:
	git clone https://github.com/Matt8898/lil.git lil

	cd lil; git am < ../0001-sone-fixes.patch

include /opt/foxos_sdk/module.mak