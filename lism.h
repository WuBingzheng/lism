#ifndef LISM_H
#define LISM_H

typedef struct {
	const char	*p;
	const char	*end;
	char		last;
} lism_ctx_t;

#define LISM_ERROR	-1
#define LISM_AGAIN	-2

static inline void lism_init(lism_ctx_t *ctx, const char *str, int len)
{
	ctx->p = str;
	ctx->end = str + len;
	ctx->last = '\0';
}

static inline void lism_extend(lism_ctx_t *ctx, int len)
{
	ctx->end += len;
}

#define LISM_MORE_OK	0
#define LISM_MORE_YES	1
#define LISM_MORE_NO	2
static inline int _lism_more(lism_ctx_t *ctx)
{
	switch (ctx->last) {
	case '(':
		return LISM_MORE_OK;
	case ')':
		if (ctx->p == ctx->end) {
			return LISM_MORE_OK;
		}
		if (ctx->p[0] == ',') {
			ctx->p++;
			return LISM_MORE_YES;
		} else {
			return LISM_MORE_NO;
		}
	default: /* '\0', '=', ',' */
		return LISM_MORE_YES;
	}
}

static inline int _lism_skip_white(lism_ctx_t *ctx)
{
	while (ctx->p < ctx->end) {
		char c = ctx->p[0];
		if (c != ' ' && c != '\t' && c != '\n') {
			return 0;
		}
		ctx->p++;
	}
	return LISM_AGAIN;
}

static inline int lism_list_open(lism_ctx_t *ctx)
{
	int more = _lism_more(ctx);

	if (_lism_skip_white(ctx) == LISM_AGAIN) {
		return LISM_AGAIN;
	}

	if (ctx->p[0] == '(') {
		if (more == LISM_MORE_NO) {
			return LISM_ERROR;
		}
		ctx->p++;
		ctx->last = '(';
		return 1;
	} else if (ctx->p[0] == ')') {
		if (more == LISM_MORE_YES) {
			return LISM_ERROR;
		}
		ctx->last = ')';
		return 0;
	}
	return LISM_ERROR;
}

static inline int lism_list_close(lism_ctx_t *ctx)
{
	if (_lism_more(ctx) == LISM_MORE_YES) {
		return LISM_ERROR;
	}
	if (_lism_skip_white(ctx) == LISM_AGAIN) {
		return LISM_AGAIN;
	}
	if (ctx->p[0] != ')') {
		return LISM_ERROR;
	}
	ctx->p++;
	ctx->last = ')';

	_lism_skip_white(ctx);
	return 1;
}

static inline int _lism_get(lism_ctx_t *ctx, char end, char invalid,
		const char **item)
{
	int more = _lism_more(ctx);

	if (_lism_skip_white(ctx) == LISM_AGAIN) {
		return LISM_AGAIN;
	}

	*item = ctx->p;
	while (ctx->p < ctx->end) {
		char c = *ctx->p;
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
			if (more == LISM_MORE_NO) {
				return LISM_ERROR;
			}
			ctx->last = end;
			return ctx->p++ - *item;
		}
		if (c == ')') {
			int len = ctx->p - *item;
			if (more != LISM_MORE_OK && ((len == 0) ^ (more == LISM_MORE_NO))) {
				return LISM_ERROR;
			}
			ctx->last = ')';
			return len;
		}
		if (c < 0x21 || c == 0x7F) { /* printable char */
			return LISM_ERROR;
		}
		ctx->p++;
	}
	ctx->p = *item;
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
