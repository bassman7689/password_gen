#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define DEFAULT_NUM_PASSWORDS 20
#define DEFAULT_PASSWORD_LENGTH 20

typedef struct random_stream_t {
    int urandom;
} random_stream;

unsigned int get_random_number_from_random_stream(random_stream *r, unsigned int start, unsigned int end) {
    unsigned int random_num;
    ssize_t result = read(r->urandom, &random_num, sizeof(random_num));
    if (result < 0) {
        fprintf(stderr, "couldn't read from /dev/urandom!\n");
    }

    return ((random_num % end) + start);
}

random_stream *build_random_stream() {
    random_stream *r = malloc(sizeof(random_stream));
    r->urandom = open("/dev/urandom", O_RDONLY);

    if (r->urandom < 0) {
        fprintf(stderr, "couldn't open /dev/urandom for reading!\n");
        exit(1);
    }

    unsigned int random_seed = get_random_number_from_random_stream(r, INT_MIN, INT_MAX);
    return r;
}

void destroy_random_stream(random_stream *r) {
    close(r->urandom);
}


typedef struct char_set_t {
    int size;
    char* characters;
    random_stream *r;
} char_set;

char_set *build_default_char_set(random_stream *r) {
    char_set* cs = malloc(sizeof(char_set));
    cs->size = 0;
    cs->r = r;

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

char get_random_char_from_char_set(char_set * cs) {
    unsigned int idx = get_random_number_from_random_stream(cs->r, 0, cs->size-1);
    assert(idx < cs->size);
    assert(idx >= 0);
    assert(isprint(cs->characters[idx]));
    return cs->characters[idx];
}

void destroy_char_set(char_set *cs) {
    assert(cs);
    assert(cs->r);
    assert(cs->characters);

    free(cs->characters);
    destroy_random_stream(cs->r);
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
    printf("passwords:\n\n");
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
    random_stream *r = build_random_stream();
    char_set* cs = build_default_char_set(r);

    int num_passwords = DEFAULT_NUM_PASSWORDS;
    int pass_len = DEFAULT_PASSWORD_LENGTH;
    password_set *ps = new_password_set(num_passwords, pass_len);
    build_password_set(cs, ps);

    print_password_set(ps);

    destroy_password_set(ps);

    destroy_char_set(cs);
}
