#include <stdio.h>
#include <string.h>

#include <curl/curl.h>
#include <json-c/json.h>


struct net_responce_t
{
    size_t size;
    int net_status;
    char *data;
};

const char targetUrl[] = "https://duckduckgo.com/js/spice/currency_convert/1/USD/KZT";

size_t curl_writer(void *ptr, size_t size, size_t nmemb, struct net_responce_t *data);

int main()
{
    CURLcode result;
    CURL *curl;
   struct net_responce_t reader = {0};
    json_object *json,*jval,*jval1;
    double convertedValue = -1.0;

    curl_global_init(CURL_GLOBAL_ALL);

    curl = curl_easy_init();

    if(curl == NULL)
    {
        printf("curl init error\n");
        return -1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, targetUrl);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reader);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_writer);

    printf("Fetching current currency value USD->KZT ...\n\n");

    result = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if(result != CURLE_OK || reader.data == NULL)
    {
        printf("curl send data fail\n");
    }
    else
    {
        // OK
        for(;;){
            json = json_tokener_parse(reader.data);
            if(json == NULL)
            {
                printf("Invalid responce is not valid json\n");
                break;
            }

            jval = json_object_object_get(json, "from");
            if(jval && json_object_get_type(jval) == json_type_string)
            {
                if(strcmp("USD", json_object_get_string(jval)))
                    printf("Is not valid check USD/KZT\n");
            }
            else
            {
                printf("Entry from is not confirmed to string\n");
            }


            jval = json_object_object_get(json, "to");
            if( jval && json_object_get_type(jval) == json_type_array && json_object_array_length(jval) > 0)
            {
                // Get the first value
                jval1 = json_object_array_get_idx(jval, 0);
                convertedValue = atof(json_object_get_string(json_object_object_get(jval1, "mid"))) ;
            }
            else
            {
                printf("Invalid entry point of the array data\n");
            }

            // Clean JSON data
            json_object_put(json);
            break;
        }
    }

    if(convertedValue != -1.0)
    {
        printf("USD -> KZT is: %f\n", convertedValue);
    }
    else
    {
        printf("Fetcing fail, value is not confirmed\n");
    }

    if(reader.data)
        free(reader.data);

    curl_global_cleanup();

    return 0;
}

size_t curl_writer(void *ptr, size_t size, size_t nmemb, struct net_responce_t *data)
{
    size_t index = data->size;
    size_t n = (size * nmemb);
    char *tmp;

    data->size += (size * nmemb);

    /* Last 1 byte for NULL terminator */
    tmp = (char *) realloc(data->data, data->size + 1);

    if(tmp)
    {
        data->data = tmp;
    }
    else
    {
        if(data->data)
        {
            free(data->data);
        }
        fprintf(stderr, "Failed to allocate memory.\n");
        return 0;
    }

    memcpy(((char *) data->data + index), ptr, n);
    data->data[data->size] = '\0';

    return n;
}
