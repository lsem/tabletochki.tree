#pragma once


void RaiseInvalidOperationException(const Tabletochki::ErrorCode::type what, const string &why="")
{
    Tabletochki::InvalidOperation invlaidOperationException;
    invlaidOperationException.what = what;
    invlaidOperationException.why = why;

    throw invlaidOperationException;
}