common conguration items to merge
notes 11.5.2020/pekka

The iocom/config folder contains JSON files which can be merged into signal or parameter setup of an IO device.
Advantage of merging is JSON from here (vs. copy/pasting into device JSON) is that device specific JSON file(s) 
can be kept shorter and more readable.

Script eosal/scripts/merge_json.py is used to merge multiple multiple JSON files and this script can be called 
from device's scripts/config_to_c_code.py file.
