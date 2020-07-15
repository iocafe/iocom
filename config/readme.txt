General purpose JSON conguration files
notes 15.7.2020/pekka

The iocom/config folder contains general purpose JSON files which can be merged into signal or parameter setup of an IO device.
Merging JSON from here (vs. copy/pasting into device JSON) keeps the device specific JSON files shorter and more readable, and
future changes to these files will be includeds.

Script eosal/scripts/merge_json.py merges multiple JSON files. The merge_json.py is usually called from device's 
scripts/config_to_c_code.py file.
