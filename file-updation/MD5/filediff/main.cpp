#include <openssl/md5.h>
#include <fstream>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <set>
#include <dirent.h>
using std::string;
using namespace std;

class MD5er {
private:
    map<string, string> file2MD5; //�ļ�����MD5���ӳ��
    string dirname;//�ļ�����
    set<string> IgnoreFile;//���������MD5�롢�Ƚϵ��ļ���

public:
    MD5er(string const str) {
        dirname = str;
        IgnoreFile.insert("log.txt");
        IgnoreFile.insert("pkgtest.csv");
    }

    void GetFileName() { //��ȡ·���µ��ļ���������file2MD5
        DIR* dp = NULL;
        dp = opendir(dirname.c_str());
        struct dirent* dirp;

        while ((dirp = readdir(dp))) {
            if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0 || IgnoreFile.find(dirp->d_name) != IgnoreFile.end())
                continue;//�����./.. ���ļ�����IgnoreFile�У�������
            file2MD5[dirp->d_name] = "";

        }
        closedir(dp);
    }

    void CodeMD5() {
        map<string, string>::iterator it;
        for (it = file2MD5.begin(); it != file2MD5.end(); it++) {
            it->second = get_file_md5(dirname + "/" + it->first); //����ÿ���ļ�����MD5��
        }
    }

    string get_file_md5(const string& file_name)
    {
        string md5value;
        ifstream file(file_name.c_str(), ifstream::binary);
        MD5_CTX md5Context;
        MD5_Init(&md5Context);

        char buf[1024 * 16];
        while (file.good()) {
            file.read(buf, sizeof(buf));
            MD5_Update(&md5Context, buf, file.gcount());
        }

        unsigned char result[MD5_DIGEST_LENGTH];
        MD5_Final(result, &md5Context);

        char hex[35];
        memset(hex, 0, sizeof(hex));
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        {
            sprintf(hex + i * 2, "%02x", result[i]);
        }
        hex[32] = '\0';
        md5value = string(hex);

        return md5value;
    }

    void addIgnoreFile(string str) {
        IgnoreFile.insert(str);
    }

    bool operator==(MD5er& dir2) {//  == ������ �ж�����MD5er�ࣨ�ļ��У����ļ�MD5���Ƿ���ͬ
        map<string, string>::iterator it;
        set<string>::iterator itset;
        set<string> allFileName;
        bool ret = true;
        for (it = this->file2MD5.begin(); it != this->file2MD5.end(); it++) {
            allFileName.insert(it->first);
        }
        for (it = dir2.file2MD5.begin(); it != dir2.file2MD5.end(); it++) {
            allFileName.insert(it->first);//�����������ļ����µ������ļ���
        }
        for (itset = allFileName.begin(); itset != allFileName.end(); itset++) {
            if (this->file2MD5.find(*itset) == this->file2MD5.end()) {
                ret = false;//�����һ���ļ����Ҳ������ļ�
            }

            else if (dir2.file2MD5.find(*(itset)) == dir2.file2MD5.end()) {
                ret = false;//����ڶ����ļ����Ҳ������ļ�
            }
            else {
                if (this->file2MD5[(*itset)] == dir2.file2MD5[(*itset)]) {
                    ret = false;//�����ļ��ж��ҵ����ļ�������MD5�벻��
                }
            }
        }

        return ret;
    }
    void PrintFile2MD5(){
        
    
    }
};

int main(int argc, char* argv[])
{
    MD5er dir1 = MD5er("C:/Users/chensiyu/source/repos/filediff/filediff/test/test1");
    MD5er dir2 = MD5er("C:/Users/chensiyu/source/repos/filediff/filediff/test/test2");
    dir1.GetFileName();
    dir2.GetFileName();
    dir1.CodeMD5();
    dir2.CodeMD5();
    dir1.PrintFile2MD5();
    dir2.PrintFile2MD5();
    cout << (dir1 == dir2);
    return 0;
}