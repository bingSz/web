#ifndef SS_FUNC_H
#define SS_FUNC_H

extern void ss_parse_config(ss_char_t *);
extern void ss_worker(void);
extern void ss_processor(ss_int_t);

extern void senderror_404(ss_int_t);
extern void senderror_502(ss_int_t);

/*hash*/
extern char *hash_search(hash_table *target, char *type);
extern void hash_insert(hash_table *target, char *type, char *descriptor);
extern int hash_init(hash_table **target);

#endif
