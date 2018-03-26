#ifndef LISM_H
#define LISM_H

typedef struct {
	const char *start;
	const char *end;
	const char *p;
} lism_ctx_t;

#define LISM_OK		0
#define LISM_ERROR	-1
#define LISM_AGAIN	-2

static inline void lism_init(lism_ctx_t *ctx, const char *str, int len)
{
	ctx->start = str;
	ctx->end = str + len;
	ctx->p = str;
}

static inline void lism_extend(lism_ctx_t *ctx, int len)
{
	ctx->end += len;
}

#define LISM_COMMA_OK		0
#define LISM_COMMA_MORE		1
#define LISM_COMMA_NOMORE	2
static inline int _lism_last_tail_comma(lism_ctx_t *ctx)
{
	if (ctx->p <= ctx->start || ctx->p[-1] != ')') {
		return LISM_COMMA_OK;
	}
	if (ctx->p[0] == ',') {
		ctx->p++;
		return LISM_COMMA_MORE;
	} else {
		return LISM_COMMA_NOMORE;
	}
}

static inline int _lism_skip_white(lism_ctx_t *ctx)
{
	while (ctx->p < ctx->end) {
		char c = ctx->p[0];
		if (c == '\0') {
			return LISM_ERROR;
		}
		if (c != ' ' && c != '\t' && c != '\n') {
			return LISM_OK;
		}
		ctx->p++;
	}
	return LISM_AGAIN;
}

static inline int lism_list_start(lism_ctx_t *ctx)
{
	if (_lism_last_tail_comma(ctx) == LISM_COMMA_NOMORE) {
		return LISM_ERROR;
	}

	int ret = _lism_skip_white(ctx);
	if (ret != LISM_OK) {
		return ret;
	}

	return *ctx->p++ == '(' ? LISM_OK : LISM_ERROR;
}

static inline int lism_list_end(lism_ctx_t *ctx)
{
	int ret = _lism_skip_white(ctx);
	if (ret != LISM_OK) {
		return ret;
	}
	return *ctx->p++ == ')' ? LISM_OK : LISM_ERROR;
}

static inline int _lism_get(lism_ctx_t *ctx, char end, char invalid,
		const char **item)
{
	int comma = _lism_last_tail_comma(ctx);

	int ret = _lism_skip_white(ctx);
	if (ret != LISM_OK) {
		return ret;
	}

	*item = ctx->p;
	int len = -1;
	while (ctx->p < ctx->end) {
		char c = *ctx->p;
		if (c == '\0') {
			return LISM_ERROR;
		}
		if (c == ' ' || c == '\t' || c == '\n') {
			return LISM_ERROR;
		}
		if (c == '(') {
			return LISM_ERROR;
		}
		if (c == invalid) {
			return LISM_ERROR;
		}
		if (c == end) {
			if (ctx->p == *item) {
				return LISM_ERROR;
			}
			if (comma == LISM_COMMA_NOMORE) {
				return LISM_ERROR;
			}
			return ctx->p++ - *item;
		}
		if (c == ')') {
			int len = ctx->p - *item;
			if (end == ',') { /* element */
				if (len == 0 || comma == LISM_COMMA_NOMORE) {
					return LISM_ERROR;
				}
			} else if (comma != LISM_COMMA_OK) { /* option */
				if ((len == 0) ^ (comma == LISM_COMMA_NOMORE)) {
					return LISM_ERROR;
				}
			}
			return len;
		}
		ctx->p++;
	}
	return LISM_AGAIN;
}

static inline int lism_element(lism_ctx_t *ctx, const char **element)
{
	return _lism_get(ctx, ',', '=', element);
}

static inline int lism_option_key(lism_ctx_t *ctx, const char **key)
{
	return _lism_get(ctx, '=', ',', key);
}

#endif
