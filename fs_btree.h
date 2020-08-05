// Copyright (c) 2020 The SorachanCoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SORACHANCOIN_FS_BTREE
#define SORACHANCOIN_FS_BTREE

#include "fs_types.h"
#include "fs_memory.h"
#include "fs_const.h"
#include "fs_datastream.h"

/*
* ** fs_btree **
*
* B-tree for Cluster (Cluster datastream + B-tree plus) on memory.
*
*/

#define INDEX_ERROR -1
#define INVALID_B_NODE ((B_NODE *)-1)
#define NO_ACCEPT_B_NODE ((B_NODE *)-2)

typedef enum _tag_node_type {
    n_unused,
    n_node,
    n_leaf,
} node_type;

typedef enum _tag_merge_status {
    m_no_connect,
    m_connected,
} merge_status;

typedef enum _tag_rem_status {
    r_node_no,
    r_node_ok,
    r_node_removed,
    r_node_need_marge,
    r_node_error,
} rem_status;

typedef struct _tag_B_NODE {
    node_type type;
    union {
        struct {
            counter_t num;
            struct _tag_NODE **node_ptr;
            index_t *begin_ptr;
        } node;
        struct {
            index_t vch_index;
        } leaf;
    } tree;
} B_NODE;

typedef struct _tag_B_INSERT {
    B_NODE **p_node;
    B_NODE **n_node;
    index_t *n_index;
} B_INSERT;

typedef enum _tag_btree_status {
    BTREE_SUCCESS = 0,
    BTREE_NO_DATA = 1,
    BTREE_ERROR_MEMORY_ALLOCATE_FAILURE = 2,
    BTREE_NO_ACCEPT = 3,
    BTREE_ERROR_TREE = 4,
} btree_status;

typedef struct _tag_FSBTREE {
    FSDATASTREAM *key;
    FSDATASTREAM *vch;
    fsize_t ksize;
    fsize_t dsize;
    B_NODE *root;
    index_t overlap_index;
    counter_t dimension;
    counter_t halfdim;
    counter_t allocnode_count;
    index_t (*fkeyequ)(const str_t *a, const str_t *b); /* equal: true_t */
    index_t (*fkeylt)(const str_t *a, const str_t *b);  /* a<=b: true_t */
    btree_status status;
} FSBTREE;

static inline index_t fs_btree_default_fkeyequ(const str_t *a, const str_t *b) {
    return strcmp(a,b)==0;
}

static inline index_t fs_btree_default_fkeylt(const str_t *a, const str_t *b) {
    return 0<=strcmp(a,b);
}

static inline void fs_btree_setfunc(FSBTREE *fbp, index_t (*fkeyequ)(const str_t *a, const str_t *b), index_t (*fkeylt)(const str_t *a, const str_t *b)) {
    if(fkeyequ) fbp->fkeyequ=fkeyequ;
    if(fkeylt) fbp->fkeylt=fkeylt;
}

static inline bool_t fs_btree_setsuccess(FSBTREE *fbp) {
    fbp->status = BTREE_SUCCESS;
    return true_t;
}

static inline bool_t fs_btree_setsuccess_nodata(FSBTREE *fbp) {
    fbp->status = BTREE_NO_DATA;
    return true_t;
}

static inline bool_t fs_btree_setsuccess_noaccept(FSBTREE *fbp) {
    fbp->status = BTREE_NO_ACCEPT;
    return true_t;
}

static inline bool_t fs_btree_seterror(FSBTREE *fbp, btree_status status) {
    fbp->status = status;
    return false_t;
}

static inline btree_status fs_btree_getstatus(FSBTREE *fbp) {
    return fbp->status;
}

static inline bool_t fs_btree_open(FSBTREE **fbp, counter_t dimension, fsize_t ksize, fsize_t dsize) { /* ksize(string): key size(bytes, include '\0'), dsize(binary): data size(bytes) */
    (*fbp) = (FSBTREE *)fs_malloc(sizeof(FSBTREE));
    if(!*fbp) return false_t;
    if(!fs_datastream_open(&(*fbp)->key)) return fs_free(*fbp, fs_btree_seterror(*fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE));
    if(!fs_datastream_open(&(*fbp)->vch)) return fs_free(*fbp, fs_datastream_close((*fbp)->key, fs_btree_seterror(*fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE)));
    (*fbp)->ksize=ksize;
    (*fbp)->dsize=dsize;
    (*fbp)->root=NULL;
    (*fbp)->overlap_index=-1;
    (*fbp)->dimension=dimension;
    (*fbp)->halfdim=(dimension+1)/2;
    (*fbp)->allocnode_count=0;
    (*fbp)->fkeyequ=&fs_btree_default_fkeyequ;
    (*fbp)->fkeylt=&fs_btree_default_fkeylt;
    return fs_btree_setsuccess(*fbp);
}

static inline index_t fs_btree_free(SRND *srnd, index_t ret) {
    return (index_t)fs_datastream_free(srnd, (bool_t)ret);
}

static inline void *fs_btree_ptrfree(SRND *srnd, void *ret) {
    fs_datastream_free(srnd, true_t);
    return ret;
}

static inline void fs_btree_debugprint(FSBTREE *fbp, const B_NODE *node, const str_t *label, const str_t *key) {
#ifdef DEBUG
    fs_printf("[%s] key:%s ",label, key);
    for(index_t i=0; i<node->tree.node.num; ++i) fs_printf("%d ", node->tree.node.begin_ptr[i]);
    fs_printf("\n");
#endif
}

static inline index_t fs_btree_getlocate(FSBTREE *fbp, const B_NODE *p, const str_t *key) { /* Note: B-tree "plus" */
    assert(2<=p->tree.node.num);
    if(p->tree.node.num==2) {
        SRND *srnd;
        if(!fs_datastream_rgetdata(fbp->key, &srnd, fbp->ksize, p->tree.node.begin_ptr[1])) return INDEX_ERROR;
        return fbp->fkeylt((const str_t *)fs_datastream_getdata(srnd), key)? 1: 0;
    } else {
        index_t left=0;
        index_t right=p->tree.node.num-1;
        while(left<right) {
            SRND *srnd0, *srnd1;
            index_t center = (left+right)>>1;
            if(!fs_datastream_rgetdata(fbp->key, &srnd0, fbp->ksize, p->tree.node.begin_ptr[center])) return INDEX_ERROR;
            index_t a = fbp->fkeylt((const str_t *)fs_datastream_getdata(srnd0), key, fbp->ksize); fs_btree_free(srnd0, true_t);
            if(!fs_datastream_rgetdata(fbp->key, &srnd1, fbp->ksize, p->tree.node.begin_ptr[center+1])) return fs_btree_free(srnd0, INDEX_ERROR);
            index_t b = fbp->fkeylt((const str_t *)fs_datastream_getdata(srnd1), key, fbp->ksize); fs_btree_free(srnd1, true_t);
            if(a&&(b==0)) return center;
            if(a==0) right=center-1;
            else left=center+1;
        }
        if(0<left) return left;
        SRND *srnd0, *srnd1;
        if(!fs_datastream_rgetdata(fbp->key, &srnd0, fbp->ksize, p->tree.node.begin_ptr[0])) return INDEX_ERROR;
        index_t a = fbp->fkeylt((const str_t *)fs_datastream_getdata(srnd0), key);
        if(!fs_datastream_rgetdata(fbp->key, &srnd1, fbp->ksize, p->tree.node.begin_ptr[1])) return fs_btree_free(srnd0, INDEX_ERROR);
        index_t b = fbp->fkeylt((const str_t *)fs_datastream_getdata(srnd1), key);
        return fs_btree_free(srnd0, fs_btree_free(srnd1, (a<b)? 1: 0));
    }
}

static inline B_NODE *fs_btree_search(FSBTREE *fbp, const str_t *key) { /* Note: B-tree "plus" */
    if(fbp->root==NULL) return NULL;
    else if(fbp->root->type==n_node) {
        const B_NODE *node=NULL;
        const B_NODE *p=fbp->root;
        index_t index=0;
        while(p->type==n_node) {
            index= fs_btree_getlocate(fbp, p, key);
            if(index==INDEX_ERROR) return INVALID_B_NODE;
            node=p;
            p=p->tree.node.node_ptr[index];
        }
        assert(node&&node->type==n_node);
        for(index_t i=0; i<node->tree.node.num; ++i) {
            p=node->tree.node.node_ptr[i];
            assert(p->type==n_leaf);
            SRND *srnd;
            if(!fs_datastream_rgetdata(fbp->key, &srnd, fbp->ksize, p->tree.leaf.vch_index)) return INDEX_ERROR;
            if(fbp->fkeyequ(key, (const str_t *)fs_datastream_getdata(srnd))) return (B_NODE *)fs_btree_ptrfree(srnd, (void *)p);
        }
        return NULL;
    } else if(fbp->root->type==n_leaf) {
        SRND *srnd;
        if(!fs_datastream_rgetdata(fbp->key, &srnd, fbp->ksize, fbp->root->tree.leaf.vch_index)) return INVALID_B_NODE;
        return fbp->fkeyequ((const str_t *)fs_datastream_getdata(srnd), key)? (B_NODE *)fs_btree_free(srnd, (void *)fbp->root): NULL;
    } else
        return INVALID_B_NODE;
}

static inline bool_t fs_btree_getdata(FSBTREE *fbp, const str_t *key, SRND **data) {
    B_NODE *node = fs_btree_search(fbp, key);
    if(node==NULL) return fs_btree_setsuccess_nodata(fbp);
    return fs_datastream_rgetdata(fbp->vch, data, fbp->dsize, node->tree.leaf.vch_index)? fs_btree_setsuccess(fbp): fs_btree_seterror(fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE);
}

static inline index_t fs_btree_getlastindex(FSBTREE *fbp) {
    return fs_datastream_rgetsize(fbp->key, fbp->ksize)-1;
}

static inline B_NODE *fs_btree_alloc(FSBTREE *fbp, node_type type, const str_t *key, const byte_t *data) {
    B_NODE *p = (B_NODE *)fs_malloc(sizeof(B_NODE));
    if(!p) return INVALID_B_NODE;
    if(type==n_leaf) { /* leaf insert */
        if(!fs_datastream_lshift(fbp->vch, data, fbp->dsize)) return INVALID_B_NODE;
        if(!fs_datastream_lshift(fbp->key, key, fbp->ksize)) return INVALID_B_NODE;
        p->type=n_leaf;
        p->tree.leaf.vch_index=fs_btree_getlastindex(fbp);
    } else if(type==n_node) { /* node insert */
        p->tree.node.node_ptr = (B_NODE **)fs_malloc(fbp->dimension*sizeof(B_NODE *));
        if(!p->tree.node.node_ptr) return INVALID_B_NODE;
        p->tree.node.begin_ptr = (index_t *)fs_malloc(fbp->dimension*sizeof(index_t));
        if(!p->tree.node.begin_ptr) return INVALID_B_NODE;
        p->type=n_node;
        memset(p->tree.node.node_ptr,NULL,fbp->dimension*sizeof(B_NODE *));
        memset(p->tree.node.begin_ptr,NULL,fbp->dimension*sizeof(index_t));
        p->tree.node.num=0;
    }
    else
        return INVALID_B_NODE;
    ++(fbp->allocnode_count);
    return p;
}

static inline B_NODE *fs_btree_insert1(FSBTREE *fbp, B_INSERT *ibp, const str_t *key, const byte_t *data) {
    *(ibp->n_node)=NULL;
    B_NODE *c_node=*(ibp->p_node);
    if(c_node->type==n_leaf) {
        B_NODE *result = fs_btree_search(fbp, key);
        if(result==INVALID_B_NODE) return INVALID_B_NODE;
        else if(result) {
            fbp->overlap_index = c_node->tree.leaf.vch_index;
            return NO_ACCEPT_B_NODE;
        } else { /* result==NULL: insert OK */
            B_NODE *alloc=fs_btree_alloc(fbp, n_leaf, key, data);
            if(alloc==INVALID_B_NODE) return INVALID_B_NODE;
            if(alloc==NO_ACCEPT_B_NODE) return NO_ACCEPT_B_NODE;
            SRND *srnd;
            if(!fs_datastream_rgetdata(fbp->key, &srnd, fbp->ksize, c_node->tree.leaf.vch_index)) return INVALID_B_NODE;
            if(fbp->fkeylt(key, (const str_t *)fs_datastream_getdata(srnd))) {
                *(ibp->p_node)=alloc;
                *(ibp->n_index)=c_node->tree.leaf.vch_index;
                *(ibp->n_node)=c_node;
            } else {
                assert((*(ibp->p_node))==c_node);
                *(ibp->n_index)=alloc->tree.leaf.vch_index;
                *(ibp->n_node)=alloc;
            }
            return (B_NODE *)fs_btree_ptrfree(srnd, alloc);
        }
    } else if(c_node->type==n_node) {
        B_NODE *xn = NULL;
        index_t	xl = 0;
        const index_t pos = fs_btree_getlocate(fbp, c_node, key);
        B_INSERT ribp={&(c_node->tree.node.node_ptr[pos]),&xn,&xl};
        B_NODE *retv = fs_btree_insert1(fbp, &ribp, key, data);
        if(retv==INVALID_B_NODE) return INVALID_B_NODE;
        if(retv==NO_ACCEPT_B_NODE) return NO_ACCEPT_B_NODE;
        if(xn==NULL) return retv;
        if(c_node->tree.node.num < fbp->dimension) {
            for(index_t i=c_node->tree.node.num-1; pos<i; --i) {
                c_node->tree.node.node_ptr[i+1] = c_node->tree.node.node_ptr[i];
                c_node->tree.node.begin_ptr[i+1] = c_node->tree.node.begin_ptr[i];
            }
            c_node->tree.node.node_ptr[pos+1]=xn;
            c_node->tree.node.begin_ptr[pos+1]=xl;
            ++(c_node->tree.node.num);
            return retv;
        } else {
            B_NODE *alloc=fs_btree_alloc(fbp, n_node, NULL, NULL);
            if(alloc==INVALID_B_NODE) return INVALID_B_NODE;
            if(alloc==NO_ACCEPT_B_NODE) return NO_ACCEPT_B_NODE;
            if(pos<fbp->halfdim-1) {
                for(index_t i=fbp->halfdim-1, j=0; i< fbp->dimension; ++i,++j) {
                    alloc->tree.node.node_ptr[j] = c_node->tree.node.node_ptr[i];
                    alloc->tree.node.begin_ptr[j] = c_node->tree.node.begin_ptr[i];
                }
                for(index_t i= fbp->halfdim-2; pos<i; --i) {
                    c_node->tree.node.node_ptr[i+1] = c_node->tree.node.node_ptr[i];
                    c_node->tree.node.begin_ptr[i+1] = c_node->tree.node.begin_ptr[i];
                }
                c_node->tree.node.node_ptr[pos+1] = xn;
                c_node->tree.node.begin_ptr[pos+1] = xl;
            } else {
                index_t j = fbp->dimension - fbp->halfdim;
                for(index_t i=fbp->dimension-1; fbp->halfdim<=i; --i) {
                    if(i==pos) {
                        alloc->tree.node.node_ptr[j] = xn;
                        alloc->tree.node.begin_ptr[j--] = xl;
                    }
                    alloc->tree.node.node_ptr[j] = c_node->tree.node.node_ptr[i];
                    alloc->tree.node.begin_ptr[j--] = c_node->tree.node.begin_ptr[i];
                }
                if(pos<fbp->halfdim) {
                    alloc->tree.node.node_ptr[0] = xn;
                    alloc->tree.node.begin_ptr[0] = xl;
                }
            }
            c_node->tree.node.num = fbp->halfdim;
            alloc->tree.node.num = (fbp->dimension+1)-fbp->halfdim;
            *(ibp->n_node) = alloc;
            *(ibp->n_index) = alloc->tree.node.begin_ptr[0];
            return retv;
        }
    } else
        assert(!"fs_btree_insert1: bug c_node->type==n_unused");
}

static inline bool_t fs_btree_insert(FSBTREE *fbp, const str_t *key, const byte_t *data) {
    if(fbp->root==NULL) {
        fbp->root = fs_btree_alloc(fbp, n_leaf, key, data);
        if(fbp->root==INVALID_B_NODE) return fs_btree_seterror(fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE);
        /* if(fbp->root==NO_ACCEPT_B_NODE) return fs_btree_seterror(fbp, BTREE_ERROR_NO_ACCEPT); */
        return fs_btree_setsuccess(fbp);
    } else {
        B_NODE *xn=NULL;
        index_t xl=0;
        B_INSERT ribp={&fbp->root,&xn,&xl};
        B_NODE *retv = fs_btree_insert1(fbp, &ribp, key, data);
        if(retv==INVALID_B_NODE) return fs_btree_seterror(fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE);
        if(retv==NO_ACCEPT_B_NODE) return fs_btree_setsuccess_noaccept(fbp);
        if(xn) {
            B_NODE *pn=fs_btree_alloc(fbp, n_node, NULL, NULL);
            if(pn==INVALID_B_NODE) return fs_btree_seterror(fbp, BTREE_ERROR_MEMORY_ALLOCATE_FAILURE);
            B_NODE *tmp=fbp->root;
            while(tmp->type==n_node) tmp=tmp->tree.node.node_ptr[0];
            assert(tmp->type==n_leaf);
            pn->tree.node.num = 2;
            pn->tree.node.node_ptr[0] = fbp->root;
            pn->tree.node.node_ptr[1] = xn;
            pn->tree.node.begin_ptr[0] = tmp->tree.leaf.vch_index;
            pn->tree.node.begin_ptr[1] = xl;
            fbp->root = pn;
        }
        return fs_btree_setsuccess(fbp);
    }
}

static inline void fs_btree_freenode(B_NODE *node) {
    if(node->type==n_node) {
        fs_free(node->tree.node.begin_ptr, true_t);
        fs_free(node->tree.node.node_ptr, true_t);
        fs_free(node, true_t);
    } else if(node->type==n_leaf)
        fs_free(node, true_t);
    else
        assert(!"fs_btree_freenode: bug node->type==n_unused");
}

static inline merge_status fs_btree_merge(FSBTREE *fbp, B_NODE *p, index_t x) {
    B_NODE *a = p->tree.node.node_ptr[x];
    B_NODE *b = p->tree.node.node_ptr[x+1];
    b->tree.node.begin_ptr[0] = p->tree.node.begin_ptr[x+1];
    const index_t an = a->tree.node.num;
    const index_t bn = b->tree.node.num;
    if(an+bn<=fbp->dimension) {
        for(index_t i=0; i<bn; ++i) {
            a->tree.node.node_ptr[i+an] = b->tree.node.node_ptr[i];
            a->tree.node.begin_ptr[i+an] = b->tree.node.begin_ptr[i];
        }
        a->tree.node.num+=bn;
        fs_btree_freenode(b);
        return m_connected;
    } else {
        index_t n=(an+bn)>>1;
        if(n < an) {
            index_t move=an-n;
            for(index_t i=bn-1; 0<=i; --i) {
                b->tree.node.node_ptr[i+move] = b->tree.node.node_ptr[i];
                b->tree.node.begin_ptr[i+move] = b->tree.node.begin_ptr[i];
            }
            for(index_t i=0; i<move; ++i) {
                b->tree.node.node_ptr[i] = a->tree.node.node_ptr[i+n];
                b->tree.node.begin_ptr[i] = a->tree.node.begin_ptr[i+n];
            }
        } else {
            int move=n-an;
            for(index_t i=0; i<move; ++i) {
                a->tree.node.node_ptr[i+an] = b->tree.node.node_ptr[i];
                a->tree.node.begin_ptr[i+an] = b->tree.node.begin_ptr[i];
            }
            for(index_t i=0; i<bn-move; ++i) {
                b->tree.node.node_ptr[i] = b->tree.node.node_ptr[i+move];
                b->tree.node.begin_ptr[i] = b->tree.node.begin_ptr[i+move];
            }
        }
        a->tree.node.num = n;
        b->tree.node.num = an + bn - n;
        p->tree.node.begin_ptr[x+1] = b->tree.node.begin_ptr[0];
        return m_no_connect;
    }
}

static inline rem_status fs_btree_remove1(FSBTREE *fbp, B_NODE *node, const str_t *key, rem_status *result) {
    *result = r_node_ok;
    if(node->type==n_leaf) {
        SRND *srnd;
        if(!fs_datastream_rgetdata(fbp->key, &srnd, fbp->ksize, node->tree.leaf.vch_index)) return r_node_error;
        if(fbp->fkeyequ((const char *)fs_datastream_getdata(srnd), key, fbp->ksize)) {
            *result = r_node_removed;
            fs_btree_freenode(node);
            return (rem_status)fs_btree_free(srnd, (rem_status)r_node_ok);
        } else
            return (rem_status)fs_btree_free(srnd, (rem_status)r_node_no);
    } else if(node->type==n_node) {
        merge_status mstatus = m_no_connect;
        rem_status rstatus = r_node_no;
        index_t pos = fs_btree_getlocate(fbp, node, key);
        rem_status retv = fs_btree_remove1(fbp, node->tree.node.node_ptr[pos], key, &rstatus);
        if(rstatus==r_node_error) return r_node_error;
        if(rstatus==r_node_ok) return retv;
        if(rstatus==r_node_need_marge) {
            int sub = (pos==0)? 0: pos-1;
            mstatus = fs_btree_merge(fbp, node, sub);
            if(mstatus==m_connected) pos=sub+1;
        }
        if(rstatus==r_node_removed || mstatus==m_connected) {
            for(index_t i=pos; i<node->tree.node.num-1; ++i) {
                node->tree.node.node_ptr[i] = node->tree.node.node_ptr[i+1];
                node->tree.node.begin_ptr[i] = node->tree.node.begin_ptr[i+1];
            }
            if(--(node->tree.node.num)<fbp->halfdim) {
                *result = r_node_need_marge;
            }
        }
        return retv;
    } else
        return r_node_error;
}

static inline bool_t fs_btree_remove(FSBTREE *fbp, const byte_t *key) {
    if(fbp->root==NULL) return fs_btree_setsuccess_nodata(fbp);
    else {
        rem_status result = r_node_no;
        rem_status retv = fs_btree_remove1(fbp, fbp->root, key, &result);
        if(result==r_node_removed) fbp->root = NULL;
        else if(result==r_node_need_marge && fbp->root->tree.node.num==1) {
            B_NODE *p = fbp->root;
            fbp->root = fbp->root->tree.node.node_ptr[0];
            fs_btree_freenode(p);
        }
        return (retv==r_node_ok)? fs_btree_setsuccess(fbp): fs_btree_setsuccess_nodata(fbp);
    }
}

static inline bool_t fs_btree_clear1(FSBTREE *fbp, B_NODE *node) {
    if(node&&node->type==n_node) {
        for(index_t i=0; i<node->tree.node.num; ++i) {
            B_NODE *next = node->tree.node.node_ptr[i];
            if(next->type==n_leaf) fs_btree_freenode(next);
            else if(next->type==n_node) {
                if(!fs_btree_clear1(fbp, next)) return fs_btree_seterror(fbp, BTREE_ERROR_TREE);
            } else assert(!"fs_btree_clear1: bug next->type==n_unused");
        }
        fs_btree_freenode(node);
    }
    return (fs_btree_getstatus(fbp)!=BTREE_ERROR_TREE)? fs_btree_setsuccess(fbp): false_t;
}

static inline bool_t fs_btree_clear(FSBTREE *fbp) {
    return fs_btree_clear1(fbp, fbp->root);
}

static inline bool_t fs_btree_close(FSBTREE *fbp, bool_t ret) {
    fs_btree_clear(fbp);
    return fs_free(fbp, fs_datastream_close(fbp->key, fs_datastream_close(fbp->vch, ret)));
}

#endif
