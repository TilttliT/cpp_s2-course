#include <cstdio>
#include <vector>
#include <cstring>

const size_t BUFF_SIZE = 4096;

int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments.\n");
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (file == nullptr) {
        fprintf(stderr, "File doesn't exist.\n");
        return 2;
    }

    size_t n = strlen(argv[2]);
    std::vector<int> pi (n);
    for (size_t i = 1; i < n; ++i) {
        int j = pi[i - 1];
        while (j > 0 && argv[2][i] != argv[2][j])
            j = pi[j - 1];
        if (argv[2][i] == argv[2][j]) ++j;
        pi[i] = j;
    }

    char buff[BUFF_SIZE];
    size_t pos = 0;
    size_t size = 0;
    bool res = false;
    int prev = 0;
    while (true) {
        if (pos == size) {
            size = fread(buff, sizeof(char), BUFF_SIZE, file);
            pos = 0;
            if (ferror(file)) {
                fprintf(stderr, "An error was received while trying to read the file.\n");
                fclose(file);
                return 3;
            }
        }
        if (size == 0) {
            break;
        }
        char ch = buff[pos++];

        int j = prev;
        while (j > 0 && ch != argv[2][j])
            j = pi[j - 1];
        if (ch == argv[2][j]) ++j;
        if (j == n) {
            res = true;
        }

        prev = j;
    }

    if (res) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }

    fclose(file);

    return 0;
}
