#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define SUNXI
#ifdef SUNXI
#define ENV_PATH "./env.cfg"
#else
#define ENV_PATH  "./boot.ini"
#endif

#define dbgprint(format,args...) ({ \
		fprintf (stderr, "[%s] <%d>-->", __func__, __LINE__); \
		fprintf(stderr, format, ##args);})

void
set_env(char *key, char *value)
{
	dbgprint("%s=%s\n", key, value);
}

int
load_env(char *data, int sz)
{
	char *s;
	int i, j, nxt = 0, str_len, env_len, cmd_len;
	char *cmd_key;
	char *cmd_value;
	char *cmd;
	int ret;
	char *addr = data;

	env_len = sz;
	for (i = 0; i < env_len; i = nxt + 1) {
		for (nxt = i; addr[nxt] != '\n'; ++nxt);
		for (j = i; addr[j] == ' '; ++j);
		if (addr[j] == '#')
			continue;
		cmd = (char *) &addr[j];
		cmd_key = cmd;
		str_len = nxt - j;

		cmd_value = strchr(cmd, '=');

		if (cmd_value != NULL) {
			cmd_len = cmd_value - cmd;
			for (j = cmd_len; cmd_key[j - 1] == ' ' && j > 0; --j);

			cmd_key[j] = '\0';
			cmd_value++;
			for (; (*cmd_value == ' ') && (*cmd_value != '\n'); cmd_value++);

			if (*cmd_value == '\n') {
				cmd_value = NULL;
			} else {
				for (j = str_len;
				     cmd[j] == ' ' && cmd[j] == '\n'; --j);
				cmd[j] = '\0';
			}
		} else {
			cmd[str_len] = '\0';
			cmd_value = NULL;
		}

		set_env(cmd_key, cmd_value);

	}

	return 0;
}

static char *
read_file(const char *fn, unsigned *sz)
{
	FILE *fp = NULL;
	char *buffer = NULL;

	fp = fopen(fn, "rw");
	if (!fp) {
		dbgprint("fopen error\n");
		exit(-1);
	}

	fseek(fp, 0, SEEK_END);
	*sz = ftell(fp);
	dbgprint("my sz is %d\n", *sz);
	rewind(fp);

	buffer = (char *) malloc(sizeof (char) * (*sz));
	if (!buffer) {
		dbgprint("malloc buffer is failed...\n");
		goto fail;
	}
	fread(buffer, 1, *sz, fp);
	if (!buffer) {
		dbgprint("fread failed...\n");
		goto fail;
	}
	//dbgprint("buffer:\n%s\n", buffer);
	fclose(fp);

	return buffer;
      fail:
	fclose(fp);
	exit(-1);
}

static void
load_env_from_file(const char *fn)
{
	char *data;
	unsigned sz;

	data = read_file(fn, &sz);

	if (data != 0) {
		load_env(data, sz);
		free(data);
	}
}

int
main()
{
	load_env_from_file(ENV_PATH);

	return 0;
}
