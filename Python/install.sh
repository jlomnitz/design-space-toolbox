#!/bin/sh
swig -python -Wall designspacetoolbox_interface.i
sudo python setup.py install
sudo cp designspacetoolboxV2.py /Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/
sudo cp designspacetoolbox_test.py /Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/