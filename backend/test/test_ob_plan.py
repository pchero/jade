import common
import ast
import json


def test_ob_plan_data_types(j_plan):
    if j_plan["uuid"] != None and isinstance(j_plan["uuid"], unicode) != True:
        print("Type error. uuid. type[%s]" % type(j_plan["uuid"]))
        return False
    
    if j_plan["name"] != None and isinstance(j_plan["name"], unicode) != True:
        print("Type error. name. type[%s]" % type(j_plan["name"]))
        return False
    
    if j_plan["detail"] != None and isinstance(j_plan["detail"], unicode) != True:
        print("Type error. detail. type[%s]" % type(j_plan["detail"]))
        return False

    
    if j_plan["tech_name"] != None and isinstance(j_plan["tech_name"], unicode) != True:
        print("Type error. tech_name. type[%s]" % type(j_plan["tech_name"]))
        return False
    
    if j_plan["trunk_name"] != None and isinstance(j_plan["trunk_name"], unicode) != True:
        print("Type error. trunk_name. type[%s]" % type(j_plan["trunk_name"]))
        return False
    
    
    if isinstance(j_plan["dial_mode"], int) != True:
        print("Type error. dial_mode. type[%s]" % type(j_plan["dial_mode"]))
        return False
    
    if isinstance(j_plan["dial_timeout"], int) != True:
        print("Type error. dial_timeout. type[%s]" % type(j_plan["dial_timeout"]))
        return False

    if isinstance(j_plan["dl_end_handle"], int) != True:
        print("Type error. dl_end_handle. type[%s]" % type(j_plan["dl_end_handle"]))
        return False

    if isinstance(j_plan["retry_delay"], int) != True:
        print("Type error. retry_delay. type[%s]" % type(j_plan["retry_delay"]))
        return False


    if j_plan["caller_id"] != None and isinstance(j_plan["caller_id"], unicode) != True:
        print("Type error. caller_id. type[%s]" % type(j_plan["caller_id"]))
        return False

    if isinstance(j_plan["service_level"], int) != True:
        print("Type error. service_level. type[%s]" % type(j_plan["service_level"]))
        return False

    if j_plan["early_media"] != None and isinstance(j_plan["early_media"], unicode) != True:
        print("Type error. early_media. type[%s]" % type(j_plan["early_media"]))
        return False

    if j_plan["codecs"] != None and isinstance(j_plan["codecs"], unicode) != True:
        print("Type error. codecs. type[%s]" % type(j_plan["codecs"]))
        return False

    if j_plan["variables"] != None and isinstance(j_plan["variables"], dict) != True:
        print("Type error. variables. type[%s]" % type(j_plan["variables"]))
        return False


    if isinstance(j_plan["max_retry_cnt_1"], int) != True:
        print("Type error. max_retry_cnt_1. type[%s]" % type(j_plan["max_retry_cnt_1"]))
        return False

    if isinstance(j_plan["max_retry_cnt_2"], int) != True:
        print("Type error. max_retry_cnt_2. type[%s]" % type(j_plan["max_retry_cnt_2"]))
        return False

    if isinstance(j_plan["max_retry_cnt_3"], int) != True:
        print("Type error. max_retry_cnt_3. type[%s]" % type(j_plan["max_retry_cnt_3"]))
        return False

    if isinstance(j_plan["max_retry_cnt_4"], int) != True:
        print("Type error. max_retry_cnt_4. type[%s]" % type(j_plan["max_retry_cnt_4"]))
        return False

    if isinstance(j_plan["max_retry_cnt_5"], int) != True:
        print("Type error. max_retry_cnt_5. type[%s]" % type(j_plan["max_retry_cnt_5"]))
        return False

    if isinstance(j_plan["max_retry_cnt_6"], int) != True:
        print("Type error. max_retry_cnt_6. type[%s]" % type(j_plan["max_retry_cnt_6"]))
        return False

    if isinstance(j_plan["max_retry_cnt_7"], int) != True:
        print("Type error. max_retry_cnt_7. type[%s]" % type(j_plan["max_retry_cnt_7"]))
        return False

    if isinstance(j_plan["max_retry_cnt_8"], int) != True:
        print("Type error. max_retry_cnt_8. type[%s]" % type(j_plan["max_retry_cnt_8"]))
        return False


    if j_plan["tm_create"] != None and isinstance(j_plan["tm_create"], unicode) != True:
        print("Type error. tm_create. type[%s]" % type(j_plan["tm_create"]))
        return False

    if j_plan["tm_update"] != None and isinstance(j_plan["tm_update"], unicode) != True:
        print("Type error. tm_update. type[%s]" % type(j_plan["tm_update"]))
        return False

    if j_plan["tm_delete"] != None and isinstance(j_plan["tm_delete"], unicode) != True:
        print("Type error. tm_delete. type[%s]" % type(j_plan["tm_delete"]))
        return False

    return True



def test_default_return_ob_plans():
    url = "127.0.0.1:8081/ob/plans"
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
    
    print("Finished test_default_return_ob_plans.")
    return True


def test_default_create_and_delete_ob_plans():
    # create new plan
    url = "127.0.0.1:8081/ob/plans"
    ret_code, ret_data = common.http_send(url, "POST", "{}")
    if ret_code != 200:
        print("Return value error")
        return False
    
    j_res = json.loads(ret_data)
    ret = test_ob_plan_data_types(j_res["result"])
    if ret == False:
        print("Could not pass type check.")
        return False
    
    # get created plan uuid
    uuid = j_res["result"]["uuid"]
    if uuid == None:
        print("Could not get created uuid.")
        return False
    
    # delete plan
    url = "127.0.0.1:8081/ob/plans/%s" % uuid
    ret_code, ret_data = common.http_send(url, "DELETE", None)
    if ret_code != 200:
        print("Return value error.")
        return False
    
    print("Finished test_default_create_and_delete_ob_plans.")
    
    return True


def test_ob_plan_create_item_check():
    url = "127.0.0.1:8081/ob/plans"
    ret_code, ret_data = common.http_send(url, "POST", "{}")
    if ret_code != 200:
        print("Return value error")
        return False
    
    # json load
    j_res = json.loads(ret_data)
    
    if "api_ver" not in j_res \
        or "result" not in j_res \
        or "statuscode" not in j_res \
        or "timestamp" not in j_res :
        print("Return value error.")
        return False
    
    # delete created plan    
    url = "127.0.0.1:8081/ob/plans/%s" % j_res["result"]["uuid"]
    ret_code, ret_data = common.http_send(url, "DELETE", None)
    if ret_code != 200:
        print("Return value error.")
        return False
    
    ret = test_ob_plan_data_types(j_res["result"])
    return ret


#### Test

print("test_ob_plan")
ret = test_default_return_ob_plans()
if ret != True:
    raise

ret = test_default_create_and_delete_ob_plans()
if ret != True:
    raise

ret = test_ob_plan_create_item_check()
if ret != True:
    raise
