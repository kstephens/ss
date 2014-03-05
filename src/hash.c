// http://en.wikipedia.org/wiki/Jenkins_hash_function
static inline
ss_word_t ss_jenkins_hash(ss_word_t hash, unsigned char *key, size_t len)
{
  size_t i;
  for ( i = 0; i < len; ++ i ) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

ss ss_hash_mix(ss a, ss b)
{
  ss_word_t r = (ss_word_t) a + (ss_word_t) b;
  r = ss_jenkins_hash(r, (void*) &a, sizeof(a));
  r = ss_jenkins_hash(r, (void*) &b, sizeof(b));
  return ss_i(r >> 2);
}

ss ss_eqQ_hash(ss x)  { return ss_hash_mix(x, (ss) 0xab871f83); }
ss ss_eqvQ_hash(ss x) { return ss_eqQ_hash(x); }
