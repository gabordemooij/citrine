rm -rf build/*
rm -rf *.egg-info
rm -rf *.so
python3 setup.py clean
python3 setup.py --verbose build
cp build/lib.linux-x86_64-cpython-37/citrine_module_nl.cpython-37m-x86_64-linux-gnu.so ../