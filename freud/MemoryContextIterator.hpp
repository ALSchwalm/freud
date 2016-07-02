#ifndef FREUD_MEMORY_CONTEXT_ITERATOR
#define FREUD_MEMORY_CONTEXT_ITERATOR

#include "freud/Alignment.hpp"
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
    MemoryContextIterator(MemoryContext& ctx, bool continuous = false)
        : m_ctx(&ctx),
          m_iter(m_ctx->mapped_regions().begin()),
          m_address(0),
          m_bytes(sizeof(typename MemObject::type)),
          m_continuous(continuous) {
        if (m_ctx->mapped_regions().size() > 0) {
            m_address = m_iter->start_address;
        }
        this->increment();
    }

    void increment() {
        while (m_iter != m_ctx->mapped_regions().end()) {
            if (m_ctx->read(m_address, m_bytes, m_iter)) {
                m_address +=
                    detail::alignment_of<typename MemObject::type>::value;
                if (m_address >= m_iter->end_address) {
                    m_iter++;
                    if (m_iter == m_ctx->mapped_regions().end())
                        break;
                    m_address = m_iter->start_address;
                }

                if (!MemObject::verify(*m_ctx, this->dereference(),
                                       m_address)) {
                    continue;
                }
                return;
            } else {
                m_iter++;
                if (m_iter == m_ctx->mapped_regions().end())
                    break;
                m_address = m_iter->start_address;
            }
        }
        reached_end_of_context();
    }

    template <typename T>
    bool equal(const MemoryContextIterator<T>& other) const {
        return m_address == other.m_address;
    }

    const typename MemObject::type& dereference() const {
        return *reinterpret_cast<const typename MemObject::type*>(
            &*m_bytes.begin());
    }

    bool continuous() const { return m_continuous; }

    address_t address() const { return m_address; }

private:
    void reached_end_of_context() {
        if (!continuous()) {
            m_address = 0;
        } else {
            m_ctx->update_regions();
            m_iter = m_ctx->mapped_regions().begin();
            if (m_ctx->mapped_regions().size() > 0) {
                m_address = m_iter->start_address;
            }
            this->increment();
        }
    }

    MemoryContext* m_ctx;
    typename std::vector<MemoryContext::MemoryRegion>::const_iterator m_iter;
    address_t m_address;
    std::vector<char> m_bytes;
    bool m_continuous;
};
}
#endif
