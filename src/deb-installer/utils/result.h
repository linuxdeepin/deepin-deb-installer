/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef RESULT_H
#define RESULT_H

template <typename T>
class Result
{
public:
    static Result<T> ok(const T &value);
    static Result<T> err(const T &value);

    Result(const bool stat, const T &value);

    bool is_ok() const
    {
        return m_ok;
    }
    T unwrap() const
    {
        return m_value;
    }

private:
    bool m_ok;
    T m_value;
};

template <typename T>
Result<T> Result<T>::ok(const T &value)
{
    return {true, value};
}

template <typename T>
Result<T> Result<T>::err(const T &value)
{
    return {false, value};
}

template <typename T>
Result<T>::Result(const bool stat, const T &value)
    : m_ok(stat)
    , m_value(value) {}

#endif  // RESULT_H
