// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2024 Mikhail Yatsenko <mikhail.yatsenko@gmail.com>

#pragma once

namespace QtLogger {

constexpr char DefaultMessagePattern[] = "%{if-category}%{category}: %{endif}"
                                         "%{message}";

constexpr char PrettyMessagePattern[] = "%{time dd.MM.yyyy hh:mm:ss.zzz} "
                                        "%{if-debug} %{endif}"
                                        "%{if-info}I%{endif}"
                                        "%{if-warning}W%{endif}"
                                        "%{if-critical}E%{endif}"
                                        "%{if-fatal}F%{endif} "
                                        "[%{category}] %{message}";

} // namespace QtLogger
