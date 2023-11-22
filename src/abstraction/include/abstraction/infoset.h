#ifndef __CLASS_INFOSET_H__
#define __CLASS_INFOSET_H__

#include <vector>
#include <string>

#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

using namespace std;

namespace poker
{
    class Infoset
    {
    public:
        vector<int> regret;
        vector<int> actionCounter;

        Infoset();
        Infoset(int actions);

        vector<float> CalculateStrategy();
        vector<float> GetFinalStrategy();
    };
} // namespace poker

//////////////////////////////////////////////////////////////////////
// non-intrusive serialization for Infoset
template <class Archive>
inline void save(
    Archive &ar,
    const poker::Infoset &t,
    const unsigned int /*file_version*/
)
{
    ar << t.actionCounter;
    ar << t.regret;
}

template <class Archive>
inline void load(
    Archive &ar,
    poker::Infoset &t,
    const unsigned int /*file_version*/
)
{
    ar >> t.actionCounter;
    ar >> t.regret;
}

// split non-intrusive serialization function member into separate
// non intrusive save/load member functions
template <class Archive>
inline void serialize(
    Archive &ar,
    poker::Infoset &t,
    const unsigned int file_version)
{
    boost::serialization::split_free(ar, t, file_version);
}

#endif
