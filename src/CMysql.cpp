#if HAVE_MYSQL_H

#include "CMysql.h"
#include <stdio.h>
#include <iostream>

namespace lce
{

    mysql_exception::mysql_exception(const string &s) : runtime_error(s) {}

    MySqlRowData::MySqlRowData(const vector<string> &data, map<string, int> &s2n)
        : _data(&data), _s2n(&s2n)
    {
    }

    MySqlRowData::MySqlRowData(const MySqlRowData &right)
        : _data(right._data), _s2n(right._s2n)
    {
    }

    void MySqlRowData::operator=(const MySqlRowData &right)
    {
        _data = right._data;
        _s2n = right._s2n;
    }

    const string &MySqlRowData::operator[](const string &s) const throw(mysql_exception)
    {
        if ((*_s2n).find(s) == (*_s2n).end())
            throw mysql_exception(s + " slopover in MySqlRowData");
        return (*_data)[(*_s2n)[s]];
    }

    MySqlBasicData::MySqlBasicData()
    {
        _affected_rows = 0;
        _nRefCount = 0;
    }

    void MySqlBasicData::RefAdd()
    {
        _nRefCount++;
    }

    void MySqlBasicData::RefSub()
    {
        if (--_nRefCount == 0)
            delete this;
    }

    void MySqlBasicData::Fields(const vector<string> &v) throw(mysql_exception)
    {
        _col.clear();
        _s2n.clear();
        for (size_t i = 0; i < v.size(); i++)
        {
            if (_s2n.find(v[i]) != _s2n.end())
            {
                throw mysql_exception(string("MySqlBasicData::Fields ") + v[i] + " duplicate");
            }
            _col.push_back(v[i]);
            _s2n[v[i]] = i;
        }
    }

    void MySqlBasicData::push_back(vector<string> &v) throw(mysql_exception)
    {
        if (v.size() == 0 || v.size() != _col.size())
        {
            throw mysql_exception("MySqlBasicData::push_back: num is not match");
        }
        _data.push_back(v);
    }

    void MySqlBasicData::genrows()
    {
        _rows.clear();
        for (size_t i = 0; i < _data.size(); i++)
            _rows.push_back(MySqlRowData(_data[i], _s2n));
    }

    MySqlData::MySqlData(const MySqlData &right)
        : _data(right._data)
    {
        _data->RefAdd();
    }

    void MySqlData::operator=(const MySqlData &right)
    {
        if (_data)
            _data->RefSub();
        _data = right._data;
        _data->RefAdd();
    }

    MySqlData::MySqlData(MySqlBasicData *data)
        : _data(data)
    {
        _data->RefAdd();
    }

    MySqlData::~MySqlData()
    {
        _data->RefSub();
    }

    const MySqlRowData &MySqlData::operator[](const size_t row) const throw(mysql_exception)
    {
        if (row >= (_data->_rows).size())
        {
            char sTmp[16];
            sprintf(sTmp, "%d", (int)row);
            throw mysql_exception(string("MySqlRowData::[") + sTmp + "] over size");
        }
        return (_data->_rows)[row];
    }

    size_t MySqlData::affected_rows() const
    {
        return _data->_affected_rows;
    }
    size_t MySqlData::num_rows() const
    {
        return _data->_data.size();
    }
    size_t MySqlData::num_fields() const
    {
        return _data->_col.size();
    }
    string MySqlData::org_name() const
    {
        return _data->_org_name;
    }
    const vector<string> &MySqlData::Fields() const
    {
        return _data->_col;
    }

    CMySql::CMySql()
    {
        _Mysql = NULL;
        _bIsConn = false;
        _port = 0;
        _connectTime = 2;
        _readTime = 2;
        _writeTime = 2;
    }

    CMySql::~CMySql()
    {
        Close();
        delete _Mysql;
        _Mysql = NULL;
    }

    void CMySql::Init(const string &host, const string &user, const string &pass, unsigned short port, const string &charSet) throw(mysql_exception)
    {
        Close();
        delete _Mysql;
        _Mysql = NULL;

        _host = host;
        _user = user;
        _pass = pass;
        _port = port;
        _charSet = charSet;

        _Mysql = new MYSQL;
        Connect();
    }

    void CMySql::use(const string &db)
    {
        _dbname = db;
        Select();
    }

    void CMySql::setConnectTime(int connectTime)
    {
        _connectTime = connectTime;
    }

    void CMySql::setReadTime(int readTime)
    {
        _readTime = readTime;
    }

    void CMySql::setWriteTime(int writeTime)
    {
        _writeTime = writeTime;
    }

    void CMySql::setCharSet(const string &cs) throw(mysql_exception)
    {
        if (!_bIsConn)
        {
            _charSet = cs;
            return;
        }

        Close();
        _charSet = cs;
        Connect();

        if (_dbname.empty())
            return;

        if (mysql_select_db(_Mysql, _dbname.c_str()))
        {
            int ret_errno = mysql_errno(_Mysql);
            Close();
            if (ret_errno == 2013 || ret_errno == 2006)
            {
                Connect();
            }

            if (mysql_select_db(_Mysql, _dbname.c_str()))
                throw mysql_exception(string("CMySql::Select: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
        }
    }

    MySqlData CMySql::query(const string &sql) throw(mysql_exception)
    {
        if (mysql_real_query(_Mysql, sql.c_str(), sql.length()))
        {
            string err(mysql_error(_Mysql) + string(":") + sql);
            int ret_errno = mysql_errno(_Mysql);
            Close();
            if (ret_errno == 2013 || ret_errno == 2006)
            {
                Connect();
                if (mysql_select_db(_Mysql, _dbname.c_str()))
                    throw mysql_exception(string("CMySql::query: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
                if (mysql_real_query(_Mysql, sql.c_str(), sql.length()))
                    throw mysql_exception(string("CMySql::query: ") + mysql_error(_Mysql) + "|" + err);
            }
            else
            {
                throw mysql_exception(string("CMySql::query: ") + err);
            }
        }

        MySqlBasicData *data = new MySqlBasicData();
        if (mysql_field_count(_Mysql) == 0)
        {
            int inum = mysql_affected_rows(_Mysql);
            string err(sql);
            if (inum < 0)
            {
                delete data;
                throw mysql_exception(string("CMySql::query: ") + mysql_error(_Mysql) + "|" + err);
            }
            data->affected_rows(inum);
            return MySqlData(data);
        }
        MYSQL_RES *pstMySqlRes = mysql_store_result(_Mysql);
        if (pstMySqlRes == NULL)
        {
            string err(sql);
            delete data;
            throw mysql_exception(string("CMySql::query: mysql_store_result is null: ") + mysql_error(_Mysql) + "|" + err);
        }

        MYSQL_FIELD *field;
        unsigned i = 0;
        vector<string> vfield;
        while ((field = mysql_fetch_field(pstMySqlRes)))
        {
            if (i == 0)
            {
                data->org_name("not support");
                i++;
            }
            vfield.push_back(field->name);
        }
        try
        {
            data->Fields(vfield);
        }
        catch (mysql_exception &e)
        {
            delete data;
            mysql_free_result(pstMySqlRes);
            throw mysql_exception(string("CMySql::query: catch mysql_exception:") + e.what());
        }

        data->clear();
        MYSQL_ROW row;
        vector<string> vrow;
        try
        {
            while ((row = mysql_fetch_row(pstMySqlRes)))
            {
                vrow.clear();
                unsigned long *lengths = mysql_fetch_lengths(pstMySqlRes);
                for (i = 0; i < vfield.size(); i++)
                {
                    string s;
                    if (row[i])
                    {
                        s = string(row[i], lengths[i]);
                    }
                    else
                    {
                        s = "";
                    }
                    vrow.push_back(s);
                }
                data->push_back(vrow);
            }
        }
        catch (mysql_exception &e)
        {
            delete data;
            mysql_free_result(pstMySqlRes);
            throw mysql_exception(string("CMySql::query: catch mysql_exception: ") + e.what());
        }
        data->genrows();
        mysql_free_result(pstMySqlRes);

        return MySqlData(data);
    }

    void CMySql::Connect()
    {
        mysql_init(_Mysql);

        if (_Mysql != NULL)
        {
            my_bool myTrue = true;
            mysql_options(_Mysql, MYSQL_OPT_RECONNECT, &myTrue);
            mysql_options(_Mysql, MYSQL_OPT_COMPRESS, 0);
            mysql_options(_Mysql, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&_connectTime);
            mysql_options(_Mysql, MYSQL_OPT_READ_TIMEOUT, (const char *)&_readTime);
            mysql_options(_Mysql, MYSQL_OPT_WRITE_TIMEOUT, (const char *)&_writeTime);
        }

        if (!_charSet.empty())
        {
            if (mysql_options(_Mysql, MYSQL_SET_CHARSET_NAME, _charSet.c_str()))
            {
                throw mysql_exception(string("CMySql::Connect: mysql_options MYSQL_SET_CHARSET_NAME ") + _charSet + ":" + mysql_error(_Mysql));
            }
        }

        if (mysql_real_connect(_Mysql, _host.c_str(), _user.c_str(), _pass.c_str(), NULL, _port, NULL, CLIENT_MULTI_STATEMENTS) == NULL)
            throw mysql_exception(string("CMySql::Connect: mysql_real_connect to ") + _host + ":" + mysql_error(_Mysql));

        _bIsConn = true;
    }

    void CMySql::Close()
    {
        if (!_bIsConn)
            return;
        mysql_close(_Mysql);
        _bIsConn = false;
    }

    void CMySql::Select()
    {
        if (!_bIsConn)
            Connect();

        if (_dbname.empty())
            return;

        if (mysql_select_db(_Mysql, _dbname.c_str()))
        {
            int ret_errno = mysql_errno(_Mysql);
            Close();
            if (ret_errno == 2013 || ret_errno == 2006)
            {
                Connect();
            }

            if (mysql_select_db(_Mysql, _dbname.c_str()))
                throw mysql_exception(string("CMySql::Select: mysql_select_db ") + _dbname + ":" + mysql_error(_Mysql));
        }
    }

    std::string CMySql::escape_string(const char *s, size_t length)
    {
        if (!_bIsConn)
            Connect();

        char *p = new char[length * 2 + 1];
        char *pp = p;
        pp += mysql_real_escape_string(_Mysql, pp, s, length);
        std::string s1(p, pp - p);
        delete[] p;
        return s1;
    }

}
#endif
