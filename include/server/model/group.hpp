#ifndef GROUP_HPP
#define GROUP_HPP

#include <string>

/**
 * @brief 群组映射类
 */
class Group
{
public:
    Group(int id = -1,
          const std::string &groupname = "",
          const std::string &groupdesc = "")
        : id_(id), groupname_(groupname), groupdesc_(groupdesc)
    {
    }

    int getId()
    {
        return id_;
    }

    std::string getGroupname()
    {
        return groupname_;
    }

    std::string getGroupdesc()
    {
        return groupdesc_;
    }

    void setId(int id)
    {
        id_ = id;
    }

    void setGroupname(const std::string &groupname)
    {
        groupname_ = groupname;
    }

    void setGroupdesc(const std::string &groupdesc)
    {
        groupdesc_ = groupdesc;
    }

private:
    int id_;
    std::string groupname_;
    std::string groupdesc_;
};

#endif