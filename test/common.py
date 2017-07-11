import pycurl
import cStringIO
import base64

def http_send(url, method, data):
    '''
    @param url: Access url.
    @param auth: Authorization info.
    @param method: [GET, POST, PUT, DELETE]
    @param data: Send data
    
    @return: code, get_data.
    '''
    
    if url is None:
        raise Exception("No url to send.")
        
    conn = pycurl.Curl()
        
    conn.setopt(pycurl.URL, url)
        
    # set response buffer
    response = cStringIO.StringIO()
    conn.setopt(pycurl.WRITEFUNCTION, response.write)
    
    # set request buffer
    if data is not None:
        conn.setopt(pycurl.POST, 1)
        conn.setopt(pycurl.POSTFIELDSIZE, len(data))
        conn.setopt(pycurl.POSTFIELDS, data)
#         conn.setopt(conn.HTTPHEADER, ["Content-Type: application/json"])
        
        
#         print "Send data[%s]" % (data)
#         conn.setopt(pycurl.POSTFIELDS, data)

    # set method    
    if method == "POST":
        conn.setopt(pycurl.POST, 1)
    elif method == "GET":
        conn.setopt(pycurl.HTTPGET, 1)
    elif method == "PUT":
        conn.setopt(pycurl.CUSTOMREQUEST, "PUT")
    elif method == "DELETE":
        conn.setopt(pycurl.CUSTOMREQUEST, "DELETE")
    else:
        raise Exception("Unsupported method.")

    # perform
    conn.perform()
    
    # return    
    return conn.getinfo(pycurl.HTTP_CODE), response.getvalue()

