#pragma once
#include <string>
#include <functional>
#include "libpq/libpq-fe.h"

#include "fmt/core.h"
#include "csv/rapidcsv.h"
#include "magic_enum/magic_enum.hpp"
#include <exception>
#include <iostream>

using LogCallback = std::function<void(const std::string&)>;
class DBApiException : public std::exception
{
public:
    DBApiException(const std::string& msg) : m_msg(msg) {}
    const char* what() const noexcept override
    {
        return m_msg.c_str();
    }
private:
    const std::string m_msg;
};
class DBApi
{
public:
    DBApi(
        const std::string& host_ip,
        const std::string& user_name,
        const std::string& password,
        const std::string& db_name,
        LogCallback log_callback=[](const std::string& log) {std::cout << log << std::endl;}
    )   :
        m_host_ip(host_ip),
        m_user_name(user_name),
        m_password(password),
        m_db_name(db_name),
        m_log_callback(log_callback)
    {
        m_con = PQsetdbLogin(m_host_ip.c_str(), "5432", "", "", m_db_name.c_str(), m_user_name.c_str(), m_password.c_str());
        if (PQstatus(m_con)!= CONNECTION_OK)
        {
            std::string err_msg = fmt::format("failed to connect to postgresql {} {} {} {}; error {}", m_host_ip, m_user_name, m_password, m_db_name, PQerrorMessage(m_con));
            m_log_callback(err_msg);
            throw DBApiException(err_msg);
        }
    }
    ~DBApi()
    {
        if (m_con)
        {   
            PQfinish(m_con);
        }
        
    }

    // TODO
    DBApi(DBApi&&) = delete;
    DBApi&& operator=(DBApi&&) = delete;
    DBApi() = delete;
    DBApi(const DBApi&) = delete;
    DBApi& operator=(const DBApi&) = delete;

    bool Execute(const std::string& sql)
    {
        if (CheckConnection() == false) return false;

        PGresult* res = PQexec(m_con, sql.c_str());
        if (PQresultStatus(res)!= PGRES_COMMAND_OK)
        {
            std::string err_msg = fmt::format("failed to execute sql {} error {}", sql, PQerrorMessage(m_con));
            m_log_callback(err_msg);
            PQclear(res);
            return false;
        }
        else 
        {
            PQclear(res);
            return true;
        }
    }
    rapidcsv::Document Select(const std::string& sql)
    {
        if (CheckConnection() == false) return rapidcsv::Document("", rapidcsv::LabelParams(-1,-1));

        PGresult* res = PQexec(m_con, sql.c_str());
        if (PQresultStatus(res) != PGRES_COMMAND_OK && PQresultStatus(res)!= PGRES_TUPLES_OK)
        {
            std::string err_msg = fmt::format("failed to execute sql {} error {}; result status {}", sql, PQerrorMessage(m_con), magic_enum::enum_name(PQresultStatus(res)));
            m_log_callback(err_msg);
            PQclear(res);
            return rapidcsv::Document("", rapidcsv::LabelParams(-1,-1));
        }
        else 
        {
            int cols = PQnfields(res);
            int rows = PQntuples(res);
            std::string csv_buf;
            for (int j = 0; j < cols - 1; j++)
            {
                csv_buf += fmt::format("{},", PQfname(res, j));
            }
            csv_buf += fmt::format("{}\n", PQfname(res, cols - 1));
            for (int i = 0; i < rows; i++)
            {
                for (int j = 0; j < cols - 1; j++)
                {
                    csv_buf += fmt::format("{},", PQgetvalue(res, i, j));
                }
                csv_buf += fmt::format("{}\n", PQgetvalue(res, i, cols - 1));
            }
            std::stringstream ss(csv_buf);
            PQclear(res);
            rapidcsv::Document doc(ss, rapidcsv::LabelParams(0, -1));
            return doc;
        }

    }
private:
    bool CheckConnection()
    {
        if (PQstatus(m_con)!= CONNECTION_OK)
        {
            PQfinish(m_con);
            m_con = PQsetdbLogin(m_host_ip.c_str(), "5432", "", "", m_db_name.c_str(), m_user_name.c_str(), m_password.c_str());
            if (PQstatus(m_con)!= CONNECTION_OK)
            {
                std::string err_msg = fmt::format("failed to connect to postgresql {} {} {} {}; error {}", m_host_ip, m_user_name, m_password, m_db_name, PQerrorMessage(m_con));
                m_log_callback(err_msg);
                return false;
            }
        }
        return true;
    }
private:
    std::string m_host_ip;
    std::string m_user_name;
    std::string m_password;
    std::string m_db_name;
    PGconn* m_con = nullptr;
    LogCallback m_log_callback;
};