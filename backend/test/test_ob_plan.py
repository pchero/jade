import common
import ast
import json



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
    
    print("Finised test_default_return_ob_plans.")
    return True


def test_create_and_delete_ob_plans():
    url = "127.0.0.1:8081/ob/plans"
    ret_code, ret_data = common.http_send(url, "POST", "{}")
    if ret_code != 200:
        print("Return value error")
        return False
    
    ret_json = json.loads(ret_data)
    
    # get created plan uuid
    uuid = ret_json["result"]["uuid"]
    if uuid == None:
        print("Could not get created uuid.")
        return False
    
    # send delete
    url = "127.0.0.1:8081/ob/plans/%s" % uuid
    ret_code, ret_data = common.http_send(url, "DELETE", None)
    if ret_code != 200:
        print("Return value error.")
        return False
    
    print("Finished test_create_and_delete_ob_plans.")
    
    return True
    


print("test_ob_plan")
ret = test_default_return_ob_plans()
if ret != True:
    raise

ret = test_create_and_delete_ob_plans()
if ret != True:
    raise

