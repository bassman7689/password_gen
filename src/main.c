#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_NUM_PASSWORDS 20
#define DEFAULT_PASSWORD_LENGTH 20

typedef struct char_set_t {
    int size;
    char* characters;
} char_set;

char_set *build_default_char_set() {
    char_set* cs = malloc(sizeof(char_set));
    cs->size = 0;

    for (char c = 0; c < 127; c++) {
        if (isprint(c)) {
            cs->size++;
        }
    }

    cs->characters = malloc(sizeof(char) * cs->size + 1);

    int i = 0;
    for (unsigned char c = 0; c <= 127; c++) {
        if (isprint(c)) {
            cs->characters[i] = c;
            i++;
        }
    }

    cs->characters[cs->size] = '\0';
    return cs;
}

void init_rand() {
    int urandom = open("/dev/urandom", O_RDONLY);

    if (urandom < 0) {
        fprintf(stderr, "couldn't open /dev/urandom for reading!\n");
        exit(1);
    }

    int randomSeed;
    ssize_t result = read(urandom, &randomSeed, sizeof(randomSeed));
    close(urandom);
    if (result < 0) {
        fprintf(stderr, "couldn't read from /dev/urandom!\n");
    }

    srand(randomSeed);
}

char get_random_char_from_char_set(char_set * cs) {
    int idx = rand() % cs->size;
    return cs->characters[idx];
}

void destroy_char_set(char_set *cs) {
    assert(cs);
    assert(cs->characters);
    free(cs->characters);
    free(cs);
}

typedef struct password_set_t {
    int num_passwords;
    int pass_len;
    char **passwords;
} password_set;

password_set *new_password_set(int num_passwords, int pass_len) {
    password_set *ps = malloc(sizeof(password_set));
    ps->passwords =  NULL;
    ps->num_passwords = num_passwords;
    ps->pass_len = pass_len;
    return ps;
}

void build_password_set(char_set *cs, password_set *ps) {
    ps->passwords = malloc(sizeof(char *) * ps->num_passwords);
    for (int i = 0; i < ps->num_passwords; i++) {
        ps->passwords[i] = malloc(sizeof(char) * ps->pass_len + 1);

        for (int j = 0; j < ps->pass_len; j++) {
                ps->passwords[i][j] = get_random_char_from_char_set(cs);
        }

        ps->passwords[i][ps->pass_len] = '\0';
    }
}

void print_password_set(password_set *ps) {
    fprintf(stderr, "passwords:\n\n");
    for(int i = 0; i < ps->num_passwords; i++) {
        printf("%s\n", ps->passwords[i]);
    }
}

void destroy_password_set(password_set *ps) {
    for(int i = 0; i < ps->num_passwords; i++) {
        free(ps->passwords[i]);
    }

    free(ps->passwords);
    free(ps);
}

int main(int argc, char* argv[]) {
    init_rand();

    char_set* cs = build_default_char_set();

    int num_passwords = DEFAULT_NUM_PASSWORDS;
    int pass_len = DEFAULT_PASSWORD_LENGTH;
    password_set *ps = new_password_set(num_passwords, pass_len);
    build_password_set(cs, ps);

    print_password_set(ps);

    destroy_password_set(ps);
    destroy_char_set(cs);
}
