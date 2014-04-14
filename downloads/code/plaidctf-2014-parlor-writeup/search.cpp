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
    if (argc < 3) {
        printf("a.out sig target\n");
        return 0;
    }
    string sig = argv[1];
    string target = argv[2];
    unsigned char firstSig[128];
    unsigned char targetSig[128];

    DigestToRaw(sig, firstSig);
    DigestToRaw(target, targetSig);
    vector<unsigned char> vtoadd = StringToVector((unsigned char *)"b\n");

    for (unsigned int i = 0; i < (1<<28); i++) {
        // if (i % 1000000 == 0) { printf("progress: %d/%d\n", i, 1<<28); }
        firstSig[0] = i & 0xff;
        firstSig[1] = (i >> 8) & 0xff;
        firstSig[2] = (i >> 16) & 0xff;
        firstSig[3] = (firstSig[3] & 0xf) | (((i >> 24) & 0xf) << 4);
        unsigned char * secondSig;
        GenerateStretchedData(18, firstSig, vtoadd, &secondSig);
        bool yes = 1;
        for (int j = 15; j >= 4; j--) {
            if (secondSig[j] != targetSig[j]) {
                yes = 0;
                break;
            }
        }
        if (yes) {
            for(unsigned int x = 0; x < 3; x++) {
                printf("%02x", firstSig[x]);
            }
            printf("%01x\n", firstSig[3] >> 4);

            break;
            for(unsigned int x = 0; x < 16; x++) {
                printf("%02x", secondSig[x]);
            }
            printf("\n");
            break;
        }
        free(secondSig);
    }
    return 0;
}

