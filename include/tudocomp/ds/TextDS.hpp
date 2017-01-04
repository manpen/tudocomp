#pragma once

#include <tudocomp/Algorithm.hpp>
#include <tudocomp/ds/IntVector.hpp>

//Defaults
#include <tudocomp/ds/SADivSufSort.hpp>
#include <tudocomp/ds/PhiFromSA.hpp>
#include <tudocomp/ds/PLCPFromPhi.hpp>
#include <tudocomp/ds/LCPFromPLCP.hpp>
#include <tudocomp/ds/ISAFromSA.hpp>

namespace tdc {

static_assert(
    std::is_same<View::value_type, uliteral_t>::value,
    "View::value_type and uliteral_t must be the same");

/// Manages text related data structures.
template<
    typename sa_t = SADivSufSort,
    typename phi_t = PhiFromSA,
    typename plcp_t = PLCPFromPhi,
    typename lcp_t = LCPFromPLCP,
    typename isa_t = ISAFromSA
>
class TextDS : public Algorithm {
public:
    using dsflags_t = unsigned int;
    static const dsflags_t SA  = 0x01;
    static const dsflags_t ISA = 0x02;
    static const dsflags_t LCP = 0x04;
    static const dsflags_t PHI = 0x08;
    static const dsflags_t PLCP = 0x10;

    using value_type = uliteral_t;
    using sa_type = sa_t;
    using phi_type = phi_t;
    using plcp_type = plcp_t;
    using lcp_type = lcp_t;
    using isa_type = isa_t;

private:
    View m_text;
    dsflags_t m_ds;

    std::unique_ptr<sa_t>  m_sa;
    std::unique_ptr<phi_t> m_phi;
    std::unique_ptr<plcp_t> m_plcp;
    std::unique_ptr<lcp_t> m_lcp;
    std::unique_ptr<isa_t> m_isa;

    template<typename ds_t>
    inline std::unique_ptr<ds_t> construct_ds(const std::string& option) {
        std::unique_ptr<ds_t> ds = std::make_unique<ds_t>(
            env().env_for_option(option));

        ds->construct(*this);
        return ds;
    }

    template<typename ds_t>
    inline const ds_t& require_ds(
        std::unique_ptr<ds_t>& p, dsflags_t flag, const std::string& option) {

        m_ds |= flag;
        if(!p) p = construct_ds<ds_t>(option);
        return *p;
    }

    template<typename ds_t>
    inline std::unique_ptr<ds_t> release_ds(std::unique_ptr<ds_t>& p, dsflags_t flag) {
        m_ds &= ~flag;
        return std::move(p);
    }

public:
    inline static Meta meta() {
        Meta m("textds", "textds");
        m.option("sa").templated<sa_t, SADivSufSort>();
        m.option("phi").templated<phi_t, PhiFromSA>();
        m.option("plcp").templated<plcp_t, PLCPFromPhi>();
        m.option("lcp").templated<lcp_t, LCPFromPLCP>();
        m.option("isa").templated<isa_t, ISAFromSA>();
        return m;
    }

    inline TextDS(Env&& env, const View& text)
        : Algorithm(std::move(env)),
          m_text(text), m_ds(0) {

        if(!m_text.ends_with(uint8_t(0))){
             throw std::logic_error(
                 "Input has no sentinel! Please make sure you declare "
                 "the compressor calling this with "
                 "`m.needs_sentinel_terminator()` in its `meta()` function."
            );
        }
    }

    inline TextDS(Env&& env, const View& text, dsflags_t flags)
        : TextDS(std::move(env), text) {

        require(flags);
    }

    inline const sa_t& require_sa() { return require_ds(m_sa, SA, "sa"); }
    inline const phi_t& require_phi() { return require_ds(m_phi, PHI, "phi"); }
    inline const plcp_t& require_plcp() { return require_ds(m_plcp, PLCP, "plcp"); }
    inline const lcp_t& require_lcp() { return require_ds(m_lcp, LCP, "lcp"); }
    inline const isa_t& require_isa() { return require_ds(m_isa, ISA, "isa"); }

    inline std::unique_ptr<sa_t> release_sa() { return release_ds(m_sa, SA); }
    inline std::unique_ptr<phi_t> release_phi() { return release_ds(m_phi, PHI); }
    inline std::unique_ptr<plcp_t> release_plcp() { return release_ds(m_plcp, PLCP); }
    inline std::unique_ptr<lcp_t> release_lcp() { return release_ds(m_lcp, LCP); }
    inline std::unique_ptr<isa_t> release_isa() { return release_ds(m_isa, ISA); }

    inline void require(dsflags_t flags) {
        m_ds = flags;

        // construct requested structures
        if(m_ds & SA) require_sa();
        if(m_ds & PHI) require_phi();
        if(m_ds & PLCP) require_plcp();
        if(m_ds & LCP) require_lcp();
        if(m_ds & ISA) require_isa();

        // release unrequested structures
        if(!(m_ds & SA)) release_sa();
        if(!(m_ds & PHI)) release_phi();
        if(!(m_ds & PLCP)) release_plcp();
        if(!(m_ds & LCP)) release_lcp();
        if(!(m_ds & ISA)) release_isa();

        // TODO: bit-compress structures afterwards
    }

    /// Accesses the input text at position i.
    inline value_type operator[](size_t i) const {
        return m_text[i];
    }

    /// Provides direct access to the input text.
    inline const value_type* text() const {
        return m_text.data();
    }

    /// Returns the size of the input text.
    inline size_t size() const {
        return m_text.size();
    }

    inline void print(std::ostream& out, size_t base) {
        size_t w = std::max(8UL, (size_t)std::log10((double)size()) + 1);
        out << std::setfill(' ');

        //Heading
        out << std::setw(w) << "i" << " | ";
        if(m_sa) out << std::setw(w) << "SA[i]" << " | ";
        if(m_phi) out << std::setw(w) << "Phi[i]" << " | ";
        if(m_plcp) out << std::setw(w) << "PLCP[i]" << " | ";
        if(m_lcp) out << std::setw(w) << "LCP[i]" << " | ";
        if(m_isa) out << std::setw(w) << "ISA[i]" << " | ";
        out << std::endl;

        //Separator
        out << std::setfill('-');
        out << std::setw(w) << "" << "-|-";
        if(m_sa) out << std::setw(w) << "" << "-|-";
        if(m_phi) out << std::setw(w) << "" << "-|-";
        if(m_plcp) out << std::setw(w) << "" << "-|-";
        if(m_lcp) out << std::setw(w) << "" << "-|-";
        if(m_isa) out << std::setw(w) << "" << "-|-";
        out << std::endl;

        //Body
        out << std::setfill(' ');
        for(size_t i = 0; i < size(); i++) {
            out << std::setw(w) << (i + base) << " | ";
            if(m_sa) out << std::setw(w) << ((*m_sa)[i] + base) << " | ";
            if(m_phi) out << std::setw(w) << (*m_phi)[i] << " | ";
            if(m_plcp) out << std::setw(w) << (*m_plcp)[i] << " | ";
            if(m_lcp) out << std::setw(w) << (*m_lcp)[i] << " | ";
            if(m_isa) out << std::setw(w) << ((*m_isa)[i] + base) << " | ";
            out << std::endl;
        }
    }
};

} //ns
