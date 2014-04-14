#include <openssl/md5.h>
#include <vector>
#include <cstring>
#include <string>
#include <unistd.h>
#include <algorithm>
#include <cstdio>
#include <cstdlib>
using namespace std;

vector<unsigned char> StringToVector(unsigned char * str)
{
    vector<unsigned char> ret;
    for(unsigned int x = 0; x < strlen((char*)str); x++)
    {
        ret.push_back(str[x]);
    }
    return ret;
}

void DigestToRaw(string hash, unsigned char * raw)
{
    transform(hash.begin(), hash.end(), hash.begin(), ::tolower);
    string alpha("0123456789abcdef");
    for(unsigned int x = 0; x < (hash.length() / 2); x++)
    {
        raw[x] = (unsigned char)((alpha.find(hash.at((x * 2))) << 4));
        raw[x] |= (unsigned char)(alpha.find(hash.at((x * 2) + 1)));
    }
}

int GenerateStretchedData(int mlen, unsigned char * hash, vector<unsigned char> added, unsigned char ** newSig)
{
    int ret = mlen + 1;
    while (ret % 64 != 56) {
        ret++;
    }

    ret += 8;

    MD5_CTX stretch;
    MD5_Init(&stretch);
    stretch.Nl = ret * 8;
    stretch.A = hash[0] | (hash[1] << 8) | (hash[2] << 16) | (hash[3] << 24);
    stretch.B = hash[4] | (hash[5] << 8) | (hash[6] << 16) | (hash[7] << 24);
    stretch.C = hash[8] | (hash[9] << 8) | (hash[10] << 16) | (hash[11] << 24);
    stretch.D = hash[12] | (hash[13] << 8) | (hash[14] << 16) | (hash[15] << 24);
    char * toadd = new char[added.size()];
    for(unsigned int x = 0; x < added.size(); x++) {
        toadd[x] = added[x];
    }
    MD5_Update(&stretch, toadd, added.size());
    *newSig = new unsigned char[16];
    MD5_Final(*newSig, &stretch);
    delete [] toadd;
    ret += added.size();
    return ret;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("a.out sig\n");
        return 0;
    }
    string sig = argv[1];
    unsigned char firstSig[128];

    DigestToRaw(sig, firstSig);

    int cnt = 0;
    for (unsigned int i = 32; i < 128; i++) {
        vector<unsigned char> vtoadd;
        vtoadd.push_back(i);
        vtoadd.push_back('\n');
        unsigned char * secondSig;
        GenerateStretchedData(18, firstSig, vtoadd, &secondSig);
        if (secondSig[15] % 2 == 0) {
            printf("%d, ", i);
            for(unsigned int x = 0; x < 16; x++) {
                printf("%02x", secondSig[x]);
            }
            printf("\n");
            cnt++;
        }
        free(secondSig);
        if (cnt > 100) {
            return 0;
        }
    }
    return 0;
}

