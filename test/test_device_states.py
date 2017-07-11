import common
import ast
import json
import subprocess


def check_device_states_data_types(j_data):    
    
    if j_data["device"] == None or isinstance(j_data["device"], str) != True:
        print("Type error. device. type[%s]" % type(j_data["device"]))
        return False
    
    
    if j_data["state"] == None or isinstance(j_data["state"], str) != True:
        print("Type error. state. type[%s]" % type(j_data["state"]))
        return False


    return True


def test_default_return_device_states():
    url = "127.0.0.1:8081/device_states"
    ret_code, ret_data = common.http_send(url, "GET", None)
    ret_json = ast.literal_eval(ret_data)
    if ret_code != 200:
        print("Return value error")
        return False

    if "api_ver" not in ret_json \
        or "timestamp" not in ret_json \
        or "statuscode" not in ret_json \
        or "result" not in ret_json:
        
        print("Return value error")
        return False
    
    
    if "list" not in ret_json["result"] :
        print("Return value error")
        return False
    
    print("Finished test_default_return_device_states.")
    return True


def test_default_return_device_states_detail_type_check():
    
    # get first id
    cmd = ["curl", "-s", "-X", "GET", "127.0.0.1:8081/device_states"]
    ret_data = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    ret_json = ast.literal_eval(ret_data)
    j_res = ret_json["result"]["list"][0]
    id = j_res["device"]

    # send request        
    cmd = ["curl", "-s", "-X", "GET", "127.0.0.1:8081/device_states/", "-d", "{\"device\": \"%s\"}" % id]
    ret_data = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    ret_json = ast.literal_eval(ret_data)
    j_res = ret_json["result"]

    ret = check_device_states_data_types(j_res)
    if ret != True:
        return False


    print("Finished test_default_return_systems_detail_type_check.")

    return True


#### Test


print("test_device_states")
ret = test_default_return_device_states()
if ret != True:
    raise
 
ret = test_default_return_device_states_detail_type_check()
if ret != True:
    raise




