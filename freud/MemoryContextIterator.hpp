#ifndef FREUD_MEMORY_CONTEXT_ITERATOR
#define FREUD_MEMORY_CONTEXT_ITERATOR

#include "freud/MemoryContext.hpp"
#include <boost/iterator/iterator_facade.hpp>

namespace freud {

template <typename MemObject>
class MemoryContextIterator
    : public boost::iterator_facade<
          MemoryContextIterator<MemObject>, typename MemObject::type,
          boost::forward_traversal_tag, const typename MemObject::type&> {
public:
    MemoryContextIterator() : m_address(0) {}
    MemoryContextIterator(MemoryContext& ctx)
        : m_ctx(&ctx),
          m_iter(ctx.m_regions.begin()),
          m_address(0),
          m_bytes(sizeof(typename MemObject::type)) {
        if (ctx.m_regions.size() > 0) {
            m_address = m_iter->start_address;
        }
        this->increment();
    }

    void increment() {
        while (m_iter != m_ctx->m_regions.end()) {
            if (m_ctx->read(m_address, m_bytes)) {
                m_address +=
                    boost::alignment_of<typename MemObject::type>::value;
                if (m_address >= m_iter->end_address) {
                    m_iter++;
                    if (m_iter == m_ctx->m_regions.end())
                        break;
                    m_address = m_iter->start_address;
                }

                if (!MemObject::verify(this->dereference(), m_address)) {
                    continue;
                }
                return;
            } else {
                m_iter++;
                if (m_iter == m_ctx->m_regions.end())
                    break;
                m_address = m_iter->start_address;
            }
        }
        m_address = 0;
    }

    bool equal(const MemoryContextIterator& other) const {
        return m_address == other.m_address;
    }

    const typename MemObject::type& dereference() const {
        return *reinterpret_cast<const typename MemObject::type*>(
            &*m_bytes.begin());
    }

private:
    MemoryContext* m_ctx;
    typename std::list<MemoryContext::MemoryRegion>::iterator m_iter;
    uint64_t m_address;
    std::vector<char> m_bytes;
};
}
#endif
