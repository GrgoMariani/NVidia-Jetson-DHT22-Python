from distutils.core import setup, Extension
MOD="C_DHT"
module=Extension(MOD, sources=["C_DHT.c", "jetsonGPIO/jetsonGPIO.c"])
setup (name = MOD, ext_modules = [module])
