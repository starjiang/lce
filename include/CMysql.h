#ifndef __LCE_MYSQL_H__
#define __LCE_MYSQL_H__

#include <map>
#include <vector>
#include <string>
#include <stdexcept>
#include <string.h>
#include <mysql.h>

using namespace std;

namespace lce
{
    struct MySqlRowData;
    struct MySqlBasicData;
    struct MySqlData;

    /////////////////////////////////////////////////////////////////////////////////////////
    // outside class
    //
    struct mysql_exception : public runtime_error
    {
        mysql_exception(const string &s);
    };

    class CMySql
    {
    public:
        void Init(const string &host, const string &user, const string &pass, unsigned short port = 3306, const string &charSet = "") throw(mysql_exception);

        void use(const string &db);

        void setConnectTime(int connectTime);

        void setReadTime(int readTime);

        void setWriteTime(int writeTime);

        MySqlData query(const string &sql) throw(mysql_exception);

        MYSQL *mysql()
        {
            return _Mysql;
        }

        void setCharSet(const string &cs) throw(mysql_exception);

        std::string escape_string(const char *s, size_t length);
        std::string escape_string(const char *s)
        {
            return escape_string(s, strlen(s));
        }
        std::string escape_string(const std::string &s)
        {
            return escape_string(s.data(), s.size());
        }

    public:
        CMySql();

        CMySql(const CMySql &);

        void operator=(const CMySql &);

        ~CMySql();

    protected:
        void Connect();
        void Close();
        void Select();

    protected:
        MYSQL *_Mysql;
        int _connectTime;
        int _readTime;
        int _writeTime;
        bool _bIsConn;
        string _dbname;
        string _host;
        string _user;
        string _pass;
        unsigned short _port;
        string _charSet;
    };

    struct MySqlData
    {

        size_t affected_rows() const;

        size_t num_rows() const;

        size_t num_fields() const;

        string org_name() const;

        const vector<string> &Fields() const;

        const MySqlRowData &operator[](const size_t row) const throw(mysql_exception);

    public:
        MySqlData(MySqlBasicData *data);
        MySqlData(const MySqlData &right);
        void operator=(const MySqlData &right);
        ~MySqlData();

    protected:
        MySqlBasicData *_data;
    };

    struct MySqlRowData
    {
    public:
        ~MySqlRowData() {}

    public:
        const string &operator[](const string &s) const throw(mysql_exception);

    public:
        MySqlRowData(const MySqlRowData &right);
        MySqlRowData(const vector<string> &data, map<string, int> &s2n);
        void operator=(const MySqlRowData &right);

    protected:
        const vector<string> *_data;
        map<string, int> *_s2n;
    };

    struct MySqlBasicData
    {
    public: // don't modify these
        vector<vector<string>> _data;
        vector<string> _col;
        map<string, int> _s2n; // fields - indx
        string _org_name;
        size_t _affected_rows;

    protected:
        friend class MySqlData;
        vector<MySqlRowData> _rows;
        size_t _nRefCount;
        void RefAdd();
        void RefSub();

    public:
        MySqlBasicData();
        MySqlBasicData(const MySqlBasicData &); // no implementation
        ~MySqlBasicData() {}                    //{cerr << "delete " << this << endl;}

        size_t affected_rows() const
        {
            return _affected_rows;
        }

        size_t num_rows() const
        {
            return _data.size();
        }

        size_t num_fields() const
        {
            return _col.size();
        }

        string org_name() const
        {
            return _org_name;
        }

        const vector<string> &Fields() const
        {
            return _col;
        }

        void org_name(const string &s)
        {
            _org_name = s;
        }

        void affected_rows(const size_t i)
        {
            _affected_rows = i;
        }

        void Fields(const vector<string> &v) throw(mysql_exception);

        void clear()
        {
            _data.clear();
        }
        void push_back(vector<string> &v) throw(mysql_exception);
        void genrows();
    };

}

#endif //
