**==警告：以下内容由代码库作者向 AI 询问并获取，使用时请万分小心，不要在不确定使用方法时用于工程代码，如有不一致请提出反馈。==**

# 使用规范与限制

基于对头文件的完整分析，整理如下。

## 一、使用规范（必须遵守）

### 1. 结构规范

```c
exception_init;          // 必须先初始化（最外层兜底）
try {
    // 可能抛异常的代码
    function1();
}
catch(ValueError) {
    exception_clear;     // 捕获后必须清理
    // 处理异常
}
try_end;                 // 每个 try/catch 块必须以此收尾
```

- `exception_init` 应作为**最外层入口**，只调用一次。
- 每个 `try` 块**必须有对应的 `try_end`**。
- **捕获到异常后必须调用 `exception_clear`**，否则 `try_end` 会把它重新 `longjmp` 抛出去。

### 2. 异常名规范

- `throw(Name)` 与 `catch(Name)` 的 **Name 必须逐字符一致**（`strcmp` 比较）。
- 大小写敏感：`throw(ValueError)` 不能被 `catch(valueError)` 捕获。
- `throw()` 空参数 → 名字为 `"Exception"`。
- `catch(Exception)` 是**万能捕获**，匹配任何异常名。

### 3. 嵌套规范

- `try` 嵌套深度必须 **< 128**（`jmp_buf[128]`）。
- 实际受限于源码静态嵌套层数，正常代码不会触及。

### 4. 资源管理规范

- 库**不负责释放资源**。try 块中申请的内存/句柄需**手动在 catch 中清理**。
- 跨 `longjmp` 的局部变量，其值可能是**未定义的**（C 标准规定：`setjmp` 之后被修改的非 `volatile` 局部变量，`longjmp` 后值不确定）。
- 需要跨跳转保持的变量应声明为 **`volatile`** 或放在 `static`/全局。

### 5. 异常传播规范

- 若异常未被当前层 catch，`try_end` 会自动向**外层 `longjmp`**，逐层上抛。
- 所有层都未捕获 → `exception_init` 兜底，打印 `unhandled exception: <name>` 并 `exit(-1)`。

### 6. 编译环境规范

- 需要支持 **C23**（`bool`/`true`/`false` 为关键字，`{}` 空初始化合法）。
- 不支持 C23 的环境需手动加 `#include <stdbool.h>` 并适配 `{}` 初始化。

---

## 二、限制（能力边界）

### 1. 语言层面限制

| 限制 | 说明 |
|---|---|
| 无自动资源清理 | `longjmp` 不做栈展开，无析构、无 RAII |
| 无类型安全 | 异常靠**字符串名字**识别，拼写错误编译期不报错 |
| 无异常对象 | 只能传递**名字**，不能携带数据/信息 |
| 无栈回溯信息 | 异常不记录抛出位置、调用栈 |

### 2. 机制层面限制

| 限制 | 说明 |
|---|---|
| 嵌套上限 128 | `jmp_buf[128]` 硬编码 |
| 非线程安全 | 全局变量 `__curr__`、`__try_point__` 等，多线程会互相干扰 |
| `volatile` 陷阱 | 跨跳转的局部变量需 `volatile`，否则值不确定 |
| 不能跨函数返回 | `jmp_buf` 对应的函数若已返回，跳转行为未定义 |

### 3. 语法层面限制

| 限制 | 说明 |
|---|---|
| 宏语法敏感 | `try`/`catch`/`try_end` 是宏，`else if` 拼接，**不能随意加 `else`、不能在 `catch` 间插语句** |
| 括号敏感 | `catch(exception)` 参数必须是**单个标识符**，`#exception` 才能正确字符串化 |
| `finally` 语义有限 | `finally` 定义为 `if(true)`，只是普通分支，非真正的 finally |
| 不能 `break`/`continue` 跳出 | 宏展开成 `if/else`，控制流受限 |

### 4. 语义层面限制

| 限制 | 说明 |
|---|---|
| 名称即类型 | 无继承关系，`catch(Exception)` 靠**硬编码名字约定**模拟基类 |
| 无重抛原语 | 重新抛出需靠 `try_end` 自动传播，或手动再 `throw` |
| 匹配是精确匹配 | 除 `"Exception"` 外，无通配、无子类型匹配 |

---

## 三、规范用法示例

```c
#include "exceptions.h"
#include <stdio.h>

void function1() {
    throw(ValueError);
}

int main() {
    exception_init;              // 最外层兜底
    try {
        function1();
    }
    catch(ValueError) {          // 精确匹配
        exception_clear;         // 必须清理
        puts("get ValueError");
    }
    catch(Exception) {           // 万能捕获（兜底）
        exception_clear;
        puts("get some exception");
    }
    try_end;                     // 必须收尾
    return 0;
}
```

---

## 四、一句话总结

> **规范**：`init` 开头、`try_end` 收尾、捕获必 `clear`、名字要一致、资源手动管、嵌套别超 128、跨跳转变量加 `volatile`。
>
> **限制**：无类型、无对象、无自动清理、非线程安全、语法受宏约束、最多 128 层。

这套约束不是缺陷，而是 **C 语言 + `setjmp/longjmp` 机制下，一个极简异常库的合理契约**。在契约内使用，结果确定、符合预期。
