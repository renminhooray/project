#include "PageProcesser.h"
#include "WebPage.h"
#include "RssParser.h"
#include "Configuration.h"

#include <sys/time.h>
#include <ErrorCheck>
#include <algorithm>
#include <fstream>
using std::ifstream;
using std::sort;

namespace wdcpp
{
PageProcesser::PageProcesser(vector<string> &filePathList, vector<WebPage> &pageList)
    : _filePathList(filePathList),
      _nonRepetivepageList(pageList)
{
    loadStopWords();
}

void PageProcesser::loadStopWords()
{
    ifstream ifs(Configuration::getInstance()->getConfigMap()["stopwords"]);
    if (!ifs)
    {
        ERROR_PRINT("can not open stop_words.utf8");
    }
    string word;
    while (ifs >> word)
        _stopWords.push_back(word);
}

/**
 *  获取网页库
 *
 *  1. 将网页文件交给 RssPraser 解析，生成未去重的 _pageList 对象
 *  2. 使用 simhash 进行网页去重，得到去重后的 _nonRepetivepageList 对象
 *  3. 对 _nonRepetivepageList 对象中每篇文章进行词频统计
 */
void PageProcesser::process()
{
    {
        struct timeval begTime, endTime;
        gettimeofday(&begTime, NULL);
        loadPageFromXML(); // 加载网页
        gettimeofday(&endTime, NULL);
        printf("loadPageFromXML take total %ld microsecends\n",
               (endTime.tv_sec - begTime.tv_sec) * 1000000 + (endTime.tv_usec - begTime.tv_usec));
    }

    {
        struct timeval begTime, endTime;
        gettimeofday(&begTime, NULL);
        cutRedundantPage(); // 网页去重
        gettimeofday(&endTime, NULL);
        printf("cutRedundantPage take total %ld microsecends\n",
               (endTime.tv_sec - begTime.tv_sec) * 1000000 + (endTime.tv_usec - begTime.tv_usec));

        // 去重结束，可以确定每篇文章的 _docID 与 _doc
        for (size_t idx = 0; idx < _nonRepetivepageList.size(); ++idx)
        {
            _nonRepetivepageList[idx].setPageID(idx);
            _nonRepetivepageList[idx].setPageDoc();
        }
    }

    // for (auto &page : _pageList)
    //     _nonRepetivepageList.push_back(page);

    {
        struct timeval begTime, endTime;
        gettimeofday(&begTime, NULL);
        // time_t now = time(NULL);
        // printf("beg countFrequence at %s", ctime(&now));
        countFrequence(); // 统计词频
        // now = time(NULL);
        // printf("end countFrequence at %s", ctime(&now));
        gettimeofday(&endTime, NULL);
        printf("countFrequence take total %ld microsecends\n",
               (endTime.tv_sec - begTime.tv_sec) * 1000000 + (endTime.tv_usec - begTime.tv_usec));
    }
}

void PageProcesser::loadPageFromXML()
{
    PageID ID = 0;
    for (auto &filePath : _filePathList)
    {
        RssPraser rssPraser(filePath.c_str());
        for (auto &item : rssPraser.getRssItems())
        {
            WebPage page(item);
            page.setPageID(ID++); // 更新 ID 便于去重时排序
            _pageList.push_back(std::move(page));
            // _pageList.push_back(str); // 存在隐式转换
        }
    }

    // _isDelete.resize(_pageList.size(), false);
}

bool cmp(const WebPage &lhs, const WebPage &rhs)
{
    size_t lhs_size = lhs.getContent().size();
    size_t rhs_size = rhs.getContent().size();
    if (lhs_size != rhs_size)
        return lhs_size > rhs_size; // content 长的较大
    else
        return lhs.getDocId() < rhs.getDocId(); // docid 大的较大
}
void PageProcesser::cutRedundantPage()
{
    sort(_pageList.begin(), _pageList.end(), cmp); // 将所有文章按 cmp 排序

    for (auto &page : _pageList)
    {
        if (!_comparePages.cut(page)) // 若无需剔除 page 则将其保存到 _nonRepetivepageList 中
            _nonRepetivepageList.push_back(page);
    }

    // using namespace std;
    // cout << "begin PageProcesser::cutRedundantPage()" << endl;
    // cout << "size = " << _pageList.size() << endl;
    // for (size_t i = 0; i < _pageList.size(); ++i)
    // {
    //     if (_isDelete[i])
    //         continue;

    //     for (size_t j = i + 1; j < _pageList.size(); ++j) // 遍历所有未 delete 的 page
    //     {
    //         if (_isDelete[j])
    //             continue;

    //         if (_pageList[i] == _pageList[j]) // operator== 在 WebPage 类中实现
    //         {

    //             cout << "they are equal" << endl;
    //             _isDelete[j] = true; // 若两个 page 相等，删除后者
    //         }
    //         else
    //         {
    //             cout << "they are not equal" << endl;
    //         }
    //     }

    //     _nonRepetivepageList.push_back(_pageList[i]); // 将前者插入 _nonRepetivepageList（真正的网页库）
    // }
    // cout << "end PageProcesser::cutRedundantPage()" << endl;
    // cout << "size = " << _nonRepetivepageList.size() << endl;
}

/**
 *  对每篇文章进行分词并统计词频
 */
void PageProcesser::countFrequence()
{
    for (auto &page : _nonRepetivepageList)
        page.splitWord(_splitTool, _stopWords);
}

void PageProcesser::printPageList()
{
    using namespace std;
    cout << "_pageList.size = " << _pageList.size() << endl;
    for (auto &page : _pageList)
    {
        cout << "doc = " << page.getDoc() << endl;
        cout << "docId = " << page.getDocId() << endl;
        cout << "docTitle = " << page.getTitle() << endl;
        cout << "docURL = " << page.getUrl() << endl;
        cout << "docContent = " << page.getContent() << endl;
    }
}

void PageProcesser::printStopWords()
{
    using namespace std;
    cout << "PageProcesser::printStopWords()" << endl;
    for (auto &word : _stopWords)
        cout << word << endl;
}
}; // namespace wdcpp
