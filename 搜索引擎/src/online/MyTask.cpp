#include "MyTask.h"
#include "MyLog.h"
#include "CacheManager.h"
#include "nlohmann/json.hpp"
#include "fifo_map.hpp"
using namespace nlohmann;
/* 以下为 nlohmann/json 库使用，保证插入顺序不变 */
template <class K, class V, class dummy_compare, class A>
using my_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using my_json = basic_json<my_workaround_fifo_map>;
using Json = my_json;

#include <ErrorCheck>

namespace wdcpp
{
extern __thread size_t __thread_id; // 工作线程的编号（0, 1, 2, ... , _workerNum-1）

void MyTask::process() // 由子线程（TheadPool）调用！！！
{

#ifdef __DEBUG__
    printf("\t(File:%s, Func:%s(), Line:%d)\n", __FILE__, __FUNCTION__, __LINE__);
    cout << _msg << endl;
#endif

    string response;

    Json root = json::parse(_msg); // 解析 _msg
    size_t msgID = root["msgID"];
    if (1 == msgID)
    {
        string word = root["msg"];
        string key = word;
        auto result = _redis.get(key); // 查询 redis
        if (result)
        {
            response = result.value();
            cout << "key hit redis: <" << word << ", ...>" << endl;
        }
        else
        {
            LogInfo("\n\tredis miss: %s", word.c_str());
            response = _recommander.doQuery(word); // 查询词典（在 doQuery 中序列化）
            _redis.setex(key, 60, response);
            cout << "key insert redis: <" << word << ", ...>" << endl;
        }
    }
    else if (2 == msgID)
    {
        string query = root["msg"];

        CacheManager *pManager = CacheManager::getInstance();
        // 查 LRU 缓存，若命中直接发送
        if ((response = pManager->getCacheGroup(__thread_id).getRecord(query)) == "")
        {
            // 若未命中
            // 将 response 插入
            LogInfo("\n\tLRU miss: %s", query.c_str());
            response = _webPageSearcher.doQuery(query);
            pManager->getCacheGroup(__thread_id).insertRecord(query, response);
            cout << "query insert LRU: <" << query << ", ...>" << endl;
        }
        else
            cout << "query hit LRU: <" << query << ", ...>" << endl;
    }
    else
    {
        ERROR_PRINT("Error: msgID = %d", msgID);
        response = "";
    }

    // send
    _connPtr->notifyLoop(response); // 注意，response 是已经序列化后的字符串
}
}; // namespace wdcpp
