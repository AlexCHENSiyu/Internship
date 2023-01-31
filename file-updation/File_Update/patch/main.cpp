#include "md5.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <set>
#include <io.h>
#include <dirent.h>
#include <direct.h>
#include <stdio.h> 
#include <Windows.h>
#include <wininet.h> 
#include <curl/curl.h>

#pragma comment (lib,"User32.lib")
#pragma comment( lib,"Urlmon.lib")
#pragma comment( lib, "wininet.lib") 
#define MAXBLOCKSIZE 1024
#define FLUSH_NUM 8
using namespace std;

#if _MSC_VER>=1900  
#include "stdio.h"   
_ACRTIMP_ALT FILE* __cdecl __acrt_iob_func(unsigned);
#ifdef __cplusplus   
extern "C"
#endif   
FILE * __cdecl __iob_func(unsigned i) {
    return __acrt_iob_func(i);
}
#endif /* _MSC_VER>=1900 */  

void PrintMD5(const string& str, MD5& md5) {
    cout << "MD5(\"" << str << "\") = " << md5.toString() << endl;
}

class MD5er {
private:
    map<string, string> file2MD5; //文件名到MD5码的映射
    string dirname; //文件夹名
    set<string> IgnoreFile; //不参与计算MD5码、比较的文件名

public:
    MD5er(const string str) {
        dirname = str;
        IgnoreFile.insert("catalog.csv");
        IgnoreFile.insert("catalog_new.csv");
        IgnoreFile.insert("diff.csv");
    }

    void GetFileName(string dir = "") { //获取路径下的文件名，放入file2MD5
        struct dirent* dirp;
        DIR* dp = NULL;
        if (dir != "")
            dp = opendir(dir.c_str());
        else 
            dp = opendir(dirname.c_str());

        while ((dirp = readdir(dp)) != NULL) {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || IgnoreFile.find(dirp->d_name) != IgnoreFile.end())
                continue;   //如果是./.. 或文件名在IgnoreFile中，则跳过
            else if (dirp->d_type == DT_REG) {   ///file
                if (dir != "")
                    file2MD5[dir.substr(dirname.length()) + "/" +dirp->d_name] = "";
                else
                    file2MD5[string("/")+ dirp->d_name] = "";
            }
            else if (dirp->d_type == DT_DIR){   ///dir
                char base[200];
                memset(base, '\0', sizeof(base));
                if (dir == "")
                    strcpy_s(base, dirname.c_str());
                else
                    strcpy_s(base, dir.c_str());
                strcat_s(base, "/");
                strcat_s(base, dirp->d_name);
                GetFileName(base);
             }
        }
        closedir(dp);
    }

    void CodeMD5() {
        map<string, string>::iterator it;
        for (it = file2MD5.begin(); it != file2MD5.end(); it++) {
            it->second = get_file_md5(dirname + "/" + it->first);   //计算每个文件名的MD5码
        }
    }

    string get_file_md5(const string& file_name)
    {
        MD5 md5value;
        ifstream file(file_name.c_str(), ifstream::binary);

        char buf[1024 * 16];
        while (file.good()) {
            file.read(buf, sizeof(buf));
            md5value.update(buf, file.gcount());
        }
        return md5value.toString();
    }

    void addIgnoreFile(string str) {
        IgnoreFile.insert(str);
    }

    void PrintFile2MD5() {
        map<string, string>::iterator it;
        for (it = file2MD5.begin(); it != file2MD5.end(); it++) {
            printf("filename: %s, md5:%s\n", (it->first).c_str(), (it->second).c_str());
        }
        //printf("dirname: %s\n", dirname.c_str());
    }

    string get_catalog() {
        // 生成此版本的目录
        ofstream outFile;
        outFile.open(dirname +'/' + "catalog.csv", ios::out);
        outFile << "Path" << ',' << "MD5" << endl;
        map<string, string>::iterator it1;
        for (it1 = this->file2MD5.begin(); it1 != this->file2MD5.end(); it1++) {
            outFile << it1->first << ',' << it1->second << endl;
        }
        outFile.close();
        return dirname +'/' + "catalog.csv";
    }

    bool operator==(MD5er& dir2) {//  == 的重载 判断两个MD5er类（文件夹）的文件MD5码是否相同
        map<string, string>::iterator it;
        set<string>::iterator itset;
        set<string> allFileName;
        bool ret = true;
        for (it = this->file2MD5.begin(); it != this->file2MD5.end(); it++) {
            allFileName.insert(it->first);
        }
        for (it = dir2.file2MD5.begin(); it != dir2.file2MD5.end(); it++) {
            allFileName.insert(it->first);//用来存两个文件夹下的所有文件名
        }
        for (itset = allFileName.begin(); itset != allFileName.end(); itset++) {
            if (this->file2MD5.find(*itset) == this->file2MD5.end()) {
                ret = false;//如果第一个文件夹找不到该文件
            }
            else if (dir2.file2MD5.find(*(itset)) == dir2.file2MD5.end()) {
                ret = false;//如果第二个文件夹找不到该文件
            }
            else {
                if (this->file2MD5[(*itset)] == dir2.file2MD5[(*itset)]) {
                    ret = false;//两个文件夹都找到该文件，但是MD5码不等
                }
            }
        }
        return ret;
    }

};




vector<vector<string>> getcsv(const string csvfile) {
    ifstream inFile(csvfile, ios::in);
    string lineStr;
    vector<vector<string>> strArray;
    while (getline(inFile, lineStr))
    {
        // 存成二维表结构
        stringstream ss(lineStr);
        string str;
        vector<string> lineArray;
        // 按照逗号分隔
        while (getline(ss, str, ','))
            lineArray.push_back(str);
        strArray.push_back(lineArray);
    }
    return strArray;
}


void wininet_download(const char* Url, const char* save_as) //将Url指向的地址的文件下载到save_as指向的本地文件
{
    byte Temp[MAXBLOCKSIZE];
    ULONG Number = 1;

    FILE* stream;
    HINTERNET hSession = InternetOpen("RookIE/1.0", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if (hSession != NULL) {
        HINTERNET handle2 = InternetOpenUrl(hSession, Url, NULL, 0, INTERNET_FLAG_DONT_CACHE, 0);
        if (handle2 != NULL) {
            if ((stream = fopen(save_as, "wb")) != NULL) {
                while (Number > 0) {
                    InternetReadFile(handle2, Temp, MAXBLOCKSIZE - 1, &Number);
                    fwrite(Temp, sizeof(char), Number, stream);
                }
                fclose(stream);
            }
            InternetCloseHandle(handle2);
            handle2 = NULL;
        }
        InternetCloseHandle(hSession);
        hSession = NULL;
    }
}


size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream) { //  libcurl write callback function
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}


int download(const char* url, const char outfilename[FILENAME_MAX]) {
    /*url:   要下载文件的url地址
       outfilename:   下载文件指定的文件名*/
    CURL* curl;
    FILE* fp;
    CURLcode res;
    /*   调用curl_global_init()初始化libcurl  */
    res = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != res){
        printf("init libcurl failed.");
        curl_global_cleanup();
        return -1;
    }
    /*  调用curl_easy_init()函数得到 easy interface型指针  */
    curl = curl_easy_init();
    if (curl) {
        fopen_s(&fp, outfilename, "wb");    //读写二进制
        /*  调用curl_easy_setopt()设置传输选项 */
        res = curl_easy_setopt(curl, CURLOPT_URL, url);
        if (res != CURLE_OK){
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        /*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
        res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        if (res != CURLE_OK){
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        /*  根据curl_easy_setopt()设置的传输选项，实现回调函数以完成用户特定任务  */
        res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        if (res != CURLE_OK){
            fclose(fp);
            curl_easy_cleanup(curl);
            return -1;
        }
        res = curl_easy_perform(curl);   // 调用curl_easy_perform()函数完成传输任务
        fclose(fp);
        /* Check for errors */
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }
        /* always cleanup */
        curl_easy_cleanup(curl);                                     // 调用curl_easy_cleanup()释放内存 
    }
    curl_global_cleanup();
    return 0;
}


bool dirExists(const string& dirName_in){
    DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
    if (ftyp == INVALID_FILE_ATTRIBUTES)
        return false;  // 路径故障
    if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
        return true;   // 文件夹存在
    return false;    // 文件夹不存在
}


int createDirectory(std::string path){
    int len = path.length();
    char tmpDirPath[256] = { 0 };
    for (int i = 0; i < len; i++) {
        tmpDirPath[i] = path[i];
        if (tmpDirPath[i] == '\\' || tmpDirPath[i] == '/') {
            if (_access(tmpDirPath, 0) == -1) {
                int ret = _mkdir(tmpDirPath);
                if (ret == -1) return ret;
            }
        }
    }
    return 0;
}


string diff(const string catalog1, const string catalog2, const string save_as) { //catalog1 相对于catalog2的变化
    ofstream outFile;
    outFile.open(save_as + '/'+"diff.csv", ios::out);
    outFile << "Path" << ',' << "MD5" << ',' << "Dirty" << endl;

    vector<vector<string>> strArray1 = getcsv(catalog1);
    vector<vector<string>> strArray2 = getcsv(catalog2);

    vector<vector<string>>::iterator it1;
    for (it1 = strArray1.begin(); it1 != strArray1.end(); it1++) {
        bool match = false;
        vector<vector<string>>::iterator it2;
        for (it2 = strArray2.begin(); it2 != strArray2.end();) {
            if ((*it1)[0] == (*it2)[0]) { //文件名及相对地址match
                if ((*it1)[1] != (*it2)[1]) {  //MD5值不同，文件被修改
                    outFile << (*it1)[0] << ',' << (*it1)[1] << ',' << "Modified" << endl;
                }
                it2 = strArray2.erase(it2);
                match = true;
                break;
            }
            else {
                it2++;
            }
        }
        if (!match) {  //增加
            outFile << (*it1)[0] << ',' << (*it1)[1] << ',' << "Add" << endl;
        }
    }
    if (strArray2.size() > 0) {//删除
        vector<vector<string>>::iterator it2;
        for (it2 = strArray2.begin(); it2 != strArray2.end(); it2++) {
            outFile << (*it2)[0] << ',' << (*it2)[1] << ',' << "Delete" << endl;
        }
    }
    outFile.close();
    return save_as + '/' + "diff.csv";
}


string localdiff(const string str1, const string str2) {  //str1是新版本， str2是旧版本
    MD5er dir1(str1);
    MD5er dir2(str2);
    dir1.GetFileName();
    dir2.GetFileName();
    dir1.CodeMD5();
    dir2.CodeMD5();
    string catalog_new = dir1.get_catalog();
    string catalog_old = dir2.get_catalog();
    //dir1.PrintFile2MD5();
    //dir2.PrintFile2MD5();

    string localdiff_file = diff(catalog_new, catalog_old, str2);
    ifstream inFile(localdiff_file, ios::in);
    string lineStr;
    while (getline(inFile, lineStr)) { // 打印整行字符串
        cout << lineStr << endl;
    }
    return localdiff_file;
}


string hostdiff(const string url, const string str) {  //url是新版本， str是旧版本
    MD5er dir(str);
    dir.GetFileName();
    dir.CodeMD5();
    string catalog_old = dir.get_catalog();
    string catalog_new = "C:/Users/chensiyu/source/repos/patch/patch/test2/catalog_new.csv";
    string catalog_url = url + '/' + "catalog.csv";
    download(catalog_url.c_str(), catalog_new.c_str());

    string hostdiff_file = diff(catalog_new, catalog_old, str);
    ifstream inFile(hostdiff_file, ios::in);
    string lineStr;
    while (getline(inFile, lineStr)){ // 打印整行字符串
        cout << lineStr << endl;
    }
    return hostdiff_file;
}


void local_update(const string str1, const string str2, const string localdiff_file) {
    vector<vector<string>> strArray = getcsv(localdiff_file);

    for (int i = 0; i < strArray.size(); i++) {  //按行读
        if (strArray[i][2] == "Delete" || strArray[i][2] == "Modified") {
            if (remove((str2 + strArray[i][0]).c_str()) != 0){
                printf("Delete %s error", (str2 + strArray[i][0]).c_str());
            }
        }
        if (strArray[i][2] == "Add" || strArray[i][2] == "Modified") {
            createDirectory(str2 + strArray[i][0]); //创建目录
            ifstream in(str1 + strArray[i][0], ios::binary);
            ofstream out(str2 + strArray[i][0], ios::binary);
            if (!in) {
                printf("open file: %s error", (str1 + strArray[i][0]).c_str());
            }
            if (!out) {
                printf("open file: %s error", (str2 + strArray[i][0]).c_str());
            }
            char flush[FLUSH_NUM];
            while (!in.eof()) {
                in.read(flush, FLUSH_NUM);
                out.write(flush, in.gcount());
            }
            in.close();
            out.close();
        }
    }
    printf("Updated");
}


void host_update(const string url, const string str, const string hostdiff_file) {
    vector<vector<string>> strArray = getcsv(hostdiff_file);

    for (int i = 0; i < strArray.size(); i++) {  //按行读
        if (strArray[i][2] == "Delete" || strArray[i][2] == "Modified") {
            if (remove((str + strArray[i][0]).c_str()) != 0) {
                printf("Delete %s error", (str + strArray[i][0]).c_str());
            }
        }
        if (strArray[i][2] == "Add" || strArray[i][2] == "Modified") {
            createDirectory(str + strArray[i][0]); //创建目录
            download((url+ strArray[i][0]).c_str(), (str+ strArray[i][0]).c_str());
        }
    }
    printf("Updated");
}


int main(int argc, char* argv[])
{
    //download("http://localhost/Tools/test1/m.txt", "C:/Users/chensiyu/source/repos/patch/patch/test1/m.txt");
    
    string str1 = "C:/Users/chensiyu/source/repos/patch/patch/test1";
    string str2 = "C:/Users/chensiyu/source/repos/patch/patch/test2";
    string url = "http://localhost/Tools/test1";

    //string localdiff_file = localdiff(str1,str2);
    //local_update(str1, str2, localdiff_file);

    string hostdiff_file = hostdiff(url, str2);
    host_update(url, str2, hostdiff_file);

    return 0;
}



