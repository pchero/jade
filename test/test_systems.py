import common
import ast
import json
import subprocess


def check_system_data_types(j_data):    
    
    if j_data["id"] == None or isinstance(j_data["id"], str) != True:
        print("Type error. id. type[%s]" % type(j_data["id"]))
        return False
    
    
    if j_data["ami_version"] == None or isinstance(j_data["ami_version"], str) != True:
        print("Type error. ami_version. type[%s]" % type(j_data["ami_version"]))
        return False
    
    if j_data["ast_version"] == None or isinstance(j_data["ast_version"], str) != True:
        print("Type error. ast_version. type[%s]" % type(j_data["ast_version"]))
        return False
    
    if j_data["system_name"] == None or isinstance(j_data["system_name"], str) != True:
        print("Type error. system_name. type[%s]" % type(j_data["system_name"]))
        return False
    
    
    if j_data["startup_date"] == None or isinstance(j_data["startup_date"], str) != True:
        print("Type error. startup_date. type[%s]" % type(j_data["startup_date"]))
        return False
    
    if j_data["startup_time"] == None or isinstance(j_data["startup_time"], str) != True:
        print("Type error. startup_time. type[%s]" % type(j_data["startup_time"]))
        return False
    
    if j_data["reload_date"] == None or isinstance(j_data["reload_date"], str) != True:
        print("Type error. reload_date. type[%s]" % type(j_data["reload_date"]))
        return False

    if j_data["reload_time"] == None or isinstance(j_data["reload_time"], str) != True:
        print("Type error. reload_time. type[%s]" % type(j_data["reload_time"]))
        return False

    if isinstance(j_data["current_calls"], int) != True:
        print("Type error. current_calls. type[%s]" % type(j_data["current_calls"]))
        return False


    if isinstance(j_data["max_calls"], int) != True:
        print("Type error. max_calls. type[%s]" % type(j_data["max_calls"]))
        return False

    if isinstance(j_data["max_load_avg"], float) != True:
        print("Type error. max_load_avg. type[%s]" % type(j_data["max_load_avg"]))
        return False

    if isinstance(j_data["max_file_handles"], int) != True:
        print("Type error. max_file_handles. type[%s]" % type(j_data["max_file_handles"]))
        return False


    if j_data["run_user"] == None or isinstance(j_data["run_user"], str) != True:
        print("Type error. run_user. type[%s]" % type(j_data["run_user"]))
        return False

    if j_data["run_group"] == None or isinstance(j_data["run_group"], str) != True:
        print("Type error. run_group. type[%s]" % type(j_data["run_group"]))
        return False


    if j_data["real_time_enabled"] == None or isinstance(j_data["real_time_enabled"], str) != True:
        print("Type error. real_time_enabled. type[%s]" % type(j_data["real_time_enabled"]))
        return False

    if j_data["cdr_enabled"] == None or isinstance(j_data["cdr_enabled"], str) != True:
        print("Type error. cdr_enabled. type[%s]" % type(j_data["cdr_enabled"]))
        return False

    if j_data["http_enabled"] == None or isinstance(j_data["http_enabled"], str) != True:
        print("Type error. http_enabled. type[%s]" % type(j_data["http_enabled"]))
        return False


    return True


def test_default_return_systems():
    url = "127.0.0.1:8081/systems"
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
    
    print("Finished test_default_return_systems.")
    return True


def test_default_return_systems():
    url = "127.0.0.1:8081/systems"
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
    
    print("Finished test_default_return_systems.")
    return True


def test_default_return_systems_detail_type_check():
    
    # get first id
    cmd = ["curl", "-s", "-X", "GET", "127.0.0.1:8081/systems"]
    ret_data = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    ret_json = ast.literal_eval(ret_data)
    j_res = ret_json["result"]["list"][0]
    id = j_res["id"]

    # send request        
    cmd = ["curl", "-s", "-X", "GET", "127.0.0.1:8081/systems/", "-d", "{\"id\": \"%s\"}" % id]
    ret_data = subprocess.check_output(cmd, stderr=subprocess.STDOUT)
    ret_json = ast.literal_eval(ret_data)
    j_res = ret_json["result"]

    ret = check_system_data_types(j_res)
    if ret != True:
        return False


    print("Finished test_default_return_systems_detail_type_check.")

    return True


#### Test


print("test_systems")
ret = test_default_return_systems()
if ret != True:
    raise
 
ret = test_default_return_systems_detail_type_check()
if ret != True:
    raise




