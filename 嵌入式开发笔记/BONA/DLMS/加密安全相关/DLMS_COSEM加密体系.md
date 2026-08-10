DLMS（Device Language Message Specification）/ COSEM 是 IEC 62056 系列标准定义的智能电表通信协议，广泛应用于电表、水表、气表等计量设备。其安全机制在 IEC 62056-5-3 中定义，覆盖认证、加密、完整性、防重放等多个维度。

# 1.整体架构

![DLMS_Security_Architecture_Overview](.\DLMS_Security_Architecture_Overview.png)

如上图所示，DLMS/COSEM 的安全体系由三大支柱构成：**安全套件**（定义算法组合）、**密钥体系**（管理各类加密密钥）、**安全服务**（提供具体的保护机制）。下面逐一深入分析。

------

# 2.安全套件

|     特性     |           Suite 0           |     Suite 1      |     Suite 2      |
| :----------: | :-------------------------: | :--------------: | :--------------: |
| **认证加密** |         AES-GCM-128         |   AES-GCM-128    |   AES-GCM-256    |
| **数字签名** |              —              |   ECDSA P-256    |   ECDSA P-384    |
| **密钥协商** |              —              |    ECDH P-256    |    ECDH P-384    |
| **哈希算法** |              —              |     SHA-256      |     SHA-384      |
| **密钥传输** | AES-128 key wrap (RFC 3394) | AES-128 key wrap | AES-256 key wrap |
|   **压缩**   |              —              |       v.44       |       v.44       |
| **密钥长度** |           128-bit           |     128-bit      |     256-bit      |

- **Suite 0**
  纯对称加密，仅使用 AES-GCM-128 + GMAC，是目前部署最广泛的方案（绝大多数 DLMS UA 认证的电表产品支持 Suite 0）
- **Suite 1**
  在 Suite 0 基础上加入非对称密码学（ECDSA/ECDH P-256 + SHA-256），支持端到端保护和前向保密
- **Suite 2**
  最强安全级别，使用 AES-256 + P-384 曲线 + SHA-384，面向高安全需求场景

------

### 二、AES-GCM 加密流程

AES-GCM（Galois/Counter Mode）是 DLMS 加密的核心算法，它同时提供**机密性**（加密）和**完整性**（认证标签）。下面是具体的加密流程：

展示详情



/* 文本类 */
.t { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 14px; fill: var(--color-text-primary, currentColor); }
.ts { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 12px; fill: var(--color-text-secondary, currentColor); }
.th { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 14px; font-weight: 500; fill: var(--color-text-primary, currentColor); }

/* 容器类 */
.box { fill: var(--color-background-secondary, #f5f5f5); stroke: var(--color-border-tertiary, #e0e0e0); stroke-width: 1; rx: 6; }
.node { cursor: pointer; }
.node:hover .box { fill: var(--color-background-tertiary, #ebebeb); }

/* 线条类 */
.arr { stroke: var(--color-text-secondary, #666); stroke-width: 1.5; fill: none; marker-end: url(#arrowhead); }
.leader { stroke: var(--color-border-tertiary, #ccc); stroke-width: 0.5; stroke-dasharray: 4 2; fill: none; }

/* 色板类 — 浅色默认值，暗色通过 .dark 祖先类覆盖 */
.c-purple { --node-bg: #EEEDFE; --node-border: #7F77DD; --node-text: #26215C; --node-text-sub: #534AB7; }
.c-teal   { --node-bg: #E1F5EE; --node-border: #1D9E75; --node-text: #04342C; --node-text-sub: #0F6E56; }
.c-coral  { --node-bg: #FAECE7; --node-border: #D85A30; --node-text: #4A1B0C; --node-text-sub: #9B3318; }
.c-pink   { --node-bg: #FBEAF0; --node-border: #D4537E; --node-text: #4B1528; --node-text-sub: #9C2D56; }
.c-gray   { --node-bg: #F1EFE8; --node-border: #888780; --node-text: #2C2C2A; --node-text-sub: #5C5C5A; }
.c-blue   { --node-bg: #E6F1FB; --node-border: #378ADD; --node-text: #042C53; --node-text-sub: #1B5C99; }
.c-green  { --node-bg: #EAF3DE; --node-border: #639922; --node-text: #173404; --node-text-sub: #3A6B10; }
.c-amber  { --node-bg: #FAEEDA; --node-border: #BA7517; --node-text: #412402; --node-text-sub: #7A4A10; }
.c-red    { --node-bg: #FCEBEB; --node-border: #E24B4A; --node-text: #501313; --node-text-sub: #9B2222; }

/* 暗色模式：覆盖色板变量（深背景 + 浅文字）

 * 双路触发：.dark 类（widget:theme 消息注入）+ prefers-color-scheme 媒体查询（不依赖消息时序）。
 * 媒体查询确保首次渲染时主题已就绪；.dark 类确保宿主显式切换主题时能覆盖系统偏好。 */
   .dark .c-purple { --node-bg: #26215C; --node-border: #7F77DD; --node-text: #EEEDFE; --node-text-sub: #B8B4F5; }
   .dark .c-teal   { --node-bg: #04342C; --node-border: #1D9E75; --node-text: #E1F5EE; --node-text-sub: #7DDDC0; }
   .dark .c-coral  { --node-bg: #4A1B0C; --node-border: #D85A30; --node-text: #FAECE7; --node-text-sub: #F0A080; }
   .dark .c-pink   { --node-bg: #4B1528; --node-border: #D4537E; --node-text: #FBEAF0; --node-text-sub: #F0A0C0; }
   .dark .c-gray   { --node-bg: #2C2C2A; --node-border: #888780; --node-text: #F1EFE8; --node-text-sub: #B8B6B0; }
   .dark .c-blue   { --node-bg: #042C53; --node-border: #378ADD; --node-text: #E6F1FB; --node-text-sub: #90C4F0; }
   .dark .c-green  { --node-bg: #173404; --node-border: #639922; --node-text: #EAF3DE; --node-text-sub: #A0D060; }
   .dark .c-amber  { --node-bg: #412402; --node-border: #BA7517; --node-text: #FAEEDA; --node-text-sub: #E0A860; }
   .dark .c-red    { --node-bg: #501313; --node-border: #E24B4A; --node-text: #FCEBEB; --node-text-sub: #F09090; }

@media (prefers-color-scheme: dark) {
    .c-purple { --node-bg: #26215C; --node-border: #7F77DD; --node-text: #EEEDFE; --node-text-sub: #B8B4F5; }
    .c-teal   { --node-bg: #04342C; --node-border: #1D9E75; --node-text: #E1F5EE; --node-text-sub: #7DDDC0; }
    .c-coral  { --node-bg: #4A1B0C; --node-border: #D85A30; --node-text: #FAECE7; --node-text-sub: #F0A080; }
    .c-pink   { --node-bg: #4B1528; --node-border: #D4537E; --node-text: #FBEAF0; --node-text-sub: #F0A0C0; }
    .c-gray   { --node-bg: #2C2C2A; --node-border: #888780; --node-text: #F1EFE8; --node-text-sub: #B8B6B0; }
    .c-blue   { --node-bg: #042C53; --node-border: #378ADD; --node-text: #E6F1FB; --node-text-sub: #90C4F0; }
    .c-green  { --node-bg: #173404; --node-border: #639922; --node-text: #EAF3DE; --node-text-sub: #A0D060; }
    .c-amber  { --node-bg: #412402; --node-border: #BA7517; --node-text: #FAEEDA; --node-text-sub: #E0A860; }
    .c-red    { --node-bg: #501313; --node-border: #E24B4A; --node-text: #FCEBEB; --node-text-sub: #F09090; }
}

/* 语义类（.box / .t / .th / .ts）消费色板变量 */
.c-purple .box, .c-teal .box, .c-coral .box, .c-pink .box, .c-gray .box,
.c-blue .box, .c-green .box, .c-amber .box, .c-red .box {
    fill: var(--node-bg); stroke: var(--node-border);
}
.c-purple .t, .c-teal .t, .c-coral .t, .c-pink .t, .c-gray .t,
.c-blue .t, .c-green .t, .c-amber .t, .c-red .t,
.c-purple .th, .c-teal .th, .c-coral .th, .c-pink .th, .c-gray .th,
.c-blue .th, .c-green .th, .c-amber .th, .c-red .th {
    fill: var(--node-text);
}
.c-purple .ts, .c-teal .ts, .c-coral .ts, .c-pink .ts, .c-gray .ts,
.c-blue .ts, .c-green .ts, .c-amber .ts, .c-red .ts {
    fill: var(--node-text-sub);
}

/* 兜底：模型直接输出 SVG 原生元素（无语义类）时自动应用色板变量。

 * 仅覆盖封闭形状（rect/circle/ellipse）；path 可能是箭头/线条，不统一覆盖。
 * SVG presentation attribute（fill=&quot;...&quot;）优先级低于 CSS，无需 !important。 */
   .c-purple rect, .c-teal rect, .c-coral rect, .c-pink rect, .c-gray rect,
   .c-blue rect, .c-green rect, .c-amber rect, .c-red rect,
   .c-purple circle, .c-teal circle, .c-coral circle, .c-pink circle, .c-gray circle,
   .c-blue circle, .c-green circle, .c-amber circle, .c-red circle,
   .c-purple ellipse, .c-teal ellipse, .c-coral ellipse, .c-pink ellipse, .c-gray ellipse,
   .c-blue ellipse, .c-green ellipse, .c-amber ellipse, .c-red ellipse {
    fill: var(--node-bg); stroke: var(--node-border);
   }
   /* text 的 fill 属性同为 presentation attribute，CSS 可直接覆盖，无需 !important */
   .c-purple text, .c-teal text, .c-coral text, .c-pink text, .c-gray text,
   .c-blue text, .c-green text, .c-amber text, .c-red text {
    fill: var(--node-text);
   }


:root {
    /* 背景色 */
    --color-background-primary: #ffffff;
    --color-background-secondary: #f5f5f5;
    --color-background-tertiary: #ebebeb;

    /* 文本色 */
    --color-text-primary: #1a1a1a;
    --color-text-secondary: #666666;
    --color-text-tertiary: #999999;
    
    /* 边框色 */
    --color-border-primary: rgba(0, 0, 0, 0.4);
    --color-border-secondary: rgba(0, 0, 0, 0.3);
    --color-border-tertiary: rgba(0, 0, 0, 0.15);
    
    /* 字体 */
    --font-sans: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    --font-serif: Georgia, 'Times New Roman', serif;
    --font-mono: 'SF Mono', Monaco, 'Cascadia Code', 'Roboto Mono', Consolas, monospace;
    
    /* 圆角 */
    --border-radius-sm: 4px;
    --border-radius-md: 8px;
    --border-radius-lg: 12px;
    --border-radius-xl: 16px;

}

.dark {
    --color-background-primary: #1a1a1a;
    --color-background-secondary: #2a2a2a;
    --color-background-tertiary: #333333;

    --color-text-primary: #e0e0e0;
    --color-text-secondary: #a0a0a0;
    --color-text-tertiary: #707070;
    
    --color-border-primary: rgba(255, 255, 255, 0.4);
    --color-border-secondary: rgba(255, 255, 255, 0.3);
    --color-border-tertiary: rgba(255, 255, 255, 0.15);

}

</style>
</head>
<body>

<div id=&quot;root&quot;></div>
<script>
var s='(function() {\n    \'use strict\';\n    var root = document.getElementById(\'root\');\n    var pendingResize = false;\n    var lastHeight = 0;\n    var currentPhase = \'bootstrap\';\n    var MIN_WIDGET_HEIGHT = 20;\n    var MAX_WIDGET_HEIGHT = 2000;\n    var CANVAS_FALLBACK_HEIGHT_PX = 320;\n    var CANVAS_MIN_HEIGHT_PX = 120;\n    var CAPTURE_MAX_HEIGHT = 16384;\n    var INLINE_PX_HEIGHT_RE = /^(\\d+(?:\\.\\d+)?)px$/;\n    var trustedTypesPolicy = null;\n\n    function initTrustedTypesPolicy() {\n        if (trustedTypesPolicy || typeof window === \'undefined\' || !window.trustedTypes || typeof window.trustedTypes.createPolicy !== \'function\') {\n            return;\n        }\n        try {\n            trustedTypesPolicy = window.trustedTypes.createPolicy(\'default\', {\n                createHTML: function(value) { return value; },\n                createScript: function(value) { return value; },\n                createScriptURL: function(value) { return value; }\n            });\n        } catch (_error) {\n            trustedTypesPolicy = null;\n        }\n    }\n\n    function toTrustedScript(code) {\n        initTrustedTypesPolicy();\n        if (trustedTypesPolicy &amp;&amp; typeof trustedTypesPolicy.createScript === \'function\') {\n            try {\n                return trustedTypesPolicy.createScript(code);\n            } catch (_error) {\n                return code;\n            }\n        }\n        return code;\n    }\n\n    function toTrustedHTML(html) {\n        initTrustedTypesPolicy();\n        if (trustedTypesPolicy &amp;&amp; typeof trustedTypesPolicy.createHTML === \'function\') {\n            try {\n                return trustedTypesPolicy.createHTML(html);\n            } catch (_error) {\n                return html;\n            }\n        }\n        return html;\n    }\n\n    function getRuntimeContext() {\n        return {\n            href: location.href,\n            userAgent: navigator.userAgent,\n            readyState: document.readyState,\n            visibilityState: document.visibilityState,\n            phase: currentPhase,\n            timestamp: new Date().toISOString()\n        };\n    }\n\n    function stringifyUnknown(value) {\n        if (value == null) {\n            return \'\';\n        }\n        if (typeof value === \'string\') {\n            return value;\n        }\n        if (typeof value === \'number\' || typeof value === \'boolean\') {\n            return String(value);\n        }\n        try {\n            return JSON.stringify(value);\n        } catch (_unusedError) {\n            return Object.prototype.toString.call(value);\n        }\n    }\n\n    function reportError(phase, message, detail, meta) {\n        var payload = {\n            type: \'widget:error\',\n            phase: phase || currentPhase,\n            message: message || \'Unknown error\',\n            detail: detail || \'\',\n            stack: meta &amp;&amp; meta.stack ? String(meta.stack) : \'\',\n            source: meta &amp;&amp; meta.source ? String(meta.source) : \'\',\n            line: meta &amp;&amp; typeof meta.line === \'number\' ? meta.line : undefined,\n            column: meta &amp;&amp; typeof meta.column === \'number\' ? meta.column : undefined,\n            runtime: getRuntimeContext(),\n            extra: meta &amp;&amp; meta.extra ? meta.extra : undefined\n        };\n        parent.postMessage(payload, \'*\');\n    }\n\n    // 全局错误捕获\n    window.onerror = function(message, source, lineno, colno, error) {\n        var fallbackDetail = source ? (source + \':\' + lineno + \':\' + colno) : \'\';\n        var stack = error &amp;&amp; error.stack ? String(error.stack) : \'\';\n        var detail = stack || fallbackDetail;\n        reportError(currentPhase, String(message || \'window.onerror\'), detail, {\n            stack: stack,\n            source: source,\n            line: typeof lineno === \'number\' ? lineno : undefined,\n            column: typeof colno === \'number\' ? colno : undefined,\n            extra: {\n                errorName: error &amp;&amp; error.name ? error.name : \'\'\n            }\n        });\n        return true;\n    };\n\n    // Promise 未捕获错误\n    window.onunhandledrejection = function(event) {\n        var reason = event &amp;&amp; event.reason;\n        var isError = reason instanceof Error;\n        var message = isError ? reason.message : stringifyUnknown(reason);\n        var stack = isError &amp;&amp; reason.stack ? String(reason.stack) : \'\';\n        var detail = stack || stringifyUnknown(reason);\n\n        reportError(currentPhase, \'Unhandled Promise rejection: \' + (message || \'unknown\'), detail, {\n            stack: stack,\n            extra: {\n                reasonType: Object.prototype.toString.call(reason)\n            }\n        });\n    };\n\n    // 全局函数：发送消息到对话\n    window.sendPrompt = function(text) {\n        if (typeof text !== \'string\') text = String(text || \'\');\n        parent.postMessage({\n            type: \'widget:sendMessage\',\n            text: text.slice(0, 500)\n        }, \'*\');\n    };\n\n    // 全局函数：打开外部链接\n    window.openLink = function(url) {\n        if (typeof url !== \'string\') return;\n        if (/^(javascript|data):/i.test(url)) return;\n        parent.postMessage({ type: \'widget:link\', href: url }, \'*\');\n    };\n\n    // 拦截链接点击\n    document.addEventListener(\'click\', function(e) {\n        var a = e.target.closest ? e.target.closest(\'a[href]\') : null;\n        if (a) {\n            e.preventDefault();\n            e.stopPropagation();\n            openLink(a.href);\n        }\n    }, true);\n\n    // 高度自适应（ResizeObserver + rAF 节流）\n    function reportHeight() {\n        if (!root) {\n            return;\n        }\n        var h = Math.ceil(root.getBoundingClientRect().height);\n        h = Math.min(Math.max(h, MIN_WIDGET_HEIGHT), MAX_WIDGET_HEIGHT);\n        if (h !== lastHeight &amp;&amp; h > 0) {\n            lastHeight = h;\n            parent.postMessage({ type: \'widget:resize\', height: h }, \'*\');\n        }\n    }\n\n    var ro = new ResizeObserver(function() {\n        if (pendingResize) return;\n        pendingResize = true;\n        requestAnimationFrame(function() {\n            pendingResize = false;\n            reportHeight();\n        });\n    });\n    if (root) {\n        ro.observe(root);\n    }\n\n    function shouldStabilizeCanvas(scripts) {\n        for (var i = 0; i < scripts.length; i++) {\n            var script = scripts[i];\n            var src = (script.getAttribute(\'src\') || \'\').toLowerCase();\n            var code = (script.textContent || \'\').toLowerCase();\n            if (src.indexOf(\'chart\') >= 0 || src.indexOf(\'chart.js\') >= 0 || code.indexOf(\'new chart(\') >= 0) {\n                return true;\n            }\n        }\n        return false;\n    }\n\n    /**\n     * 高度来源优先级：父容器 inline px -> canvas height 属性 -> 实测高度 -> 兜底高度，\n     * 最终 clamp 到 [CANVAS_MIN_HEIGHT_PX, MAX_WIDGET_HEIGHT]。\n     */\n    function stabilizeCanvasContainers(scope, enable) {\n        if (!enable) {\n            return;\n        }\n\n        var canvases = scope.querySelectorAll(\'canvas\');\n        for (var i = 0; i < canvases.length; i++) {\n            var canvas = canvases[i];\n            if (!canvas || canvas.getAttribute(\'data-widget-canvas-stable\') === \'1\') {\n                continue;\n            }\n\n            var parent = canvas.parentElement;\n            if (!parent) {\n                continue;\n            }\n\n            var parentInlineHeight = NaN;\n            if (parent.style &amp;&amp; typeof parent.style.height === \'string\') {\n                var parentHeightStr = parent.style.height.trim();\n                var parentMatch = INLINE_PX_HEIGHT_RE.exec(parentHeightStr);\n                if (parentMatch) {\n                    parentInlineHeight = parseFloat(parentMatch[1]);\n                }\n            }\n            var heightAttr = parseInt(canvas.getAttribute(\'height\') || \'\', 10);\n            var measured = Math.ceil(canvas.getBoundingClientRect ? canvas.getBoundingClientRect().height : 0);\n            var stableHeight = Number.isFinite(parentInlineHeight) &amp;&amp; parentInlineHeight > 0 ? parentInlineHeight :\n                Number.isFinite(heightAttr) &amp;&amp; heightAttr > 0 ? heightAttr :\n                measured > 0 ? measured :\n                CANVAS_FALLBACK_HEIGHT_PX;\n            stableHeight = Math.min(Math.max(stableHeight, CANVAS_MIN_HEIGHT_PX), MAX_WIDGET_HEIGHT);\n\n            var wrapper = document.createElement(\'div\');\n            wrapper.className = \'widget-canvas-shell\';\n            wrapper.style.position = \'relative\';\n            wrapper.style.width = \'100%\';\n            wrapper.style.height = stableHeight + \'px\';\n            wrapper.style.minHeight = stableHeight + \'px\';\n            wrapper.style.maxHeight = stableHeight + \'px\';\n            wrapper.style.overflow = \'hidden\';\n\n            parent.insertBefore(wrapper, canvas);\n            wrapper.appendChild(canvas);\n\n            canvas.style.width = \'100%\';\n            canvas.style.height = \'100%\';\n            canvas.style.display = \'block\';\n            canvas.setAttribute(\'data-widget-canvas-stable\', \'1\');\n        }\n    }\n\n    function bindInlineHandlers(scope) {\n        var elements = scope.querySelectorAll(\'*\');\n        for (var i = 0; i < elements.length; i++) {\n            var element = elements[i];\n            var attrs = element.getAttributeNames ? element.getAttributeNames() : [];\n            for (var j = 0; j < attrs.length; j++) {\n                var attr = attrs[j];\n                if (!/^on/i.test(attr)) continue;\n                var eventName = attr.slice(2).toLowerCase();\n                var code = element.getAttribute(attr);\n                if (!eventName || !code) continue;\n\n                element.removeAttribute(attr);\n                element.addEventListener(eventName, function(event) {\n                    var target = event.currentTarget;\n                    var handlerCode = target &amp;&amp; target.__widgetInlineHandlerCode &amp;&amp; target.__widgetInlineHandlerCode[event.type];\n                    if (!handlerCode) return;\n                    try {\n                        var handler = new Function(\'event\', toTrustedScript(handlerCode));\n                        handler.call(target, event);\n                    } catch (error) {\n                        console.warn(\'[WidgetSandbox] inline handler execution failed.\', error);\n                        reportError(\'script\', \'Inline handler failed: \' + error.message, error.stack || error.message, {\n                            stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                            extra: {\n                                eventType: event &amp;&amp; event.type ? event.type : \'\'\n                            }\n                        });\n                    }\n                });\n\n                if (!element.__widgetInlineHandlerCode) {\n                    element.__widgetInlineHandlerCode = {};\n                }\n                element.__widgetInlineHandlerCode[eventName] = code;\n            }\n        }\n    }\n\n    async function executeScriptsSequentially(scripts) {\n        currentPhase = \'script\';\n        for (var i = 0; i < scripts.length; i++) {\n            var script = scripts[i];\n            var src = script.getAttribute(\'src\');\n            var code = script.textContent || \'\';\n\n            try {\n                if (src) {\n                    var response = await fetch(src);\n                    if (!response.ok) throw new Error(\'fetch failed: \' + response.status);\n                    var externalCode = await response.text();\n                    (0, eval)(toTrustedScript(externalCode));\n                } else if (code.trim()) {\n                    (0, eval)(toTrustedScript(code));\n                }\n            } catch (error) {\n                console.warn(\'[WidgetSandbox] script execution failed.\', { src: src, error: error });\n                reportError(\'script\', \'Script execution failed\' + (src ? \' (\' + src + \')\' : \'\'), error.stack || error.message, {\n                    stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                    source: src || \'\',\n                    extra: {\n                        scriptType: src ? \'external\' : \'inline\'\n                    }\n                });\n            }\n        }\n    }\n\n    // 同步元素属性：仅写入有变化的属性、移除新内容里不存在的属性，\n    // 避免无谓的 setAttribute 触发重排。\n    function morphElementAttributes(fromEl, toEl) {\n        var toAttrs = toEl.attributes;\n        for (var i = 0; i < toAttrs.length; i++) {\n            var attr = toAttrs[i];\n            if (fromEl.getAttribute(attr.name) !== attr.value) {\n                fromEl.setAttribute(attr.name, attr.value);\n            }\n        }\n        var fromAttrs = fromEl.attributes;\n        for (var j = fromAttrs.length - 1; j >= 0; j--) {\n            var name = fromAttrs[j].name;\n            if (!toEl.hasAttribute(name)) {\n                fromEl.removeAttribute(name);\n            }\n        }\n    }\n\n    // 增量同步单个节点：类型/标签相同则原地更新，不同则整体替换。\n    function morphNode(fromNode, toNode) {\n        if (fromNode.nodeType !== toNode.nodeType || fromNode.nodeName !== toNode.nodeName) {\n            fromNode.parentNode.replaceChild(toNode, fromNode);\n            return;\n        }\n        // 文本 / 注释：内容变化时才改 nodeValue，不重建节点\n        if (fromNode.nodeType === 3 || fromNode.nodeType === 8) {\n            if (fromNode.nodeValue !== toNode.nodeValue) {\n                fromNode.nodeValue = toNode.nodeValue;\n            }\n            return;\n        }\n        if (fromNode.nodeType !== 1) {\n            return;\n        }\n        // 保留已存在的 canvas 节点（含已绘制内容与稳定高度），避免清空画布导致闪烁/图表丢失\n        if (fromNode.nodeName === \'CANVAS\') {\n            return;\n        }\n        morphElementAttributes(fromNode, toNode);\n        morphChildren(fromNode, toNode);\n    }\n\n    // 增量同步子节点列表：按位置逐个 morph，保留未变节点，仅删除/新增差异节点。\n    // 取代「整体清空再重建」，消除 finalize/update 切换时的清空闪白。\n    function morphChildren(fromEl, toEl) {\n        var toNodes = Array.prototype.slice.call(toEl.childNodes);\n        for (var i = 0; i < toNodes.length; i++) {\n            var toNode = toNodes[i];\n            var fromNode = fromEl.childNodes[i];\n            if (!fromNode) {\n                fromEl.appendChild(toNode);\n            } else {\n                morphNode(fromNode, toNode);\n            }\n        }\n        while (fromEl.childNodes.length > toNodes.length) {\n            fromEl.removeChild(fromEl.lastChild);\n        }\n    }\n\n    // 渲染前校验：判断 temp（已解析的离屏内容）是否为「可正常渲染的有效 HTML」。\n    // 流式中间帧偶发：上游内容尚未解码（实体转义残留如 &amp;lt;div&amp;gt;，或换行/制表符的字面量转义残片），\n    // 此时 innerHTML 会把整段当作纯文本，morph 进画面后出现乱码、随后被正确帧纠正，造成闪烁。\n    // 返回 false 时调用方跳过本帧、保留上一帧已渲染内容。\n    // 注意：本函数被嵌入模板字符串注入到 iframe，正则中的反斜杠需在源码里双写以保留到注入脚本。\n    function looksRenderable(sourceHtml, parsed) {\n        var src = sourceHtml || \'\';\n        if (!src) {\n            return false;\n        }\n\n        // (A) 未解码 JSON 转义残留检测 —— 必须在 hasElement 判断之前。\n        // 上游 JSON 字符串尚未解码时，HTML 里会残留「反斜杠+引号」(属性引号未解码)\n        // 或「反斜杠+ n/t/r」(字面换行、制表)。正常 HTML 用裸引号和真实空白，绝不出现这些组合。\n        // 这类脏帧仍能解析出真实元素节点，若直接 morph 进画面，残留字符会被当作可见文本画出来，\n        // 下一帧解码后再纠正 -> 表现为闪烁。出现 >=2 处即判定整帧不可渲染，保留上一帧。\n        // 用 charCode 遍历而非正则：本函数注入到 iframe 模板字符串，正则里的反斜杠转义极易写错。\n        var BACKSLASH_CODE = 92;\n        var undecodedHits = 0;\n        for (var ci = 0; ci < src.length - 1; ci++) {\n            if (src.charCodeAt(ci) === BACKSLASH_CODE) {\n                var nextChar = src.charAt(ci + 1);\n                if (nextChar === \'&quot;\' || nextChar === \'n\' || nextChar === \'t\' || nextChar === \'r\') {\n                    undecodedHits++;\n                    if (undecodedHits >= 2) {\n                        return false;\n                    }\n                }\n            }\n        }\n\n        // (B) 已解析出真实元素结构 -> 视为可渲染（不误伤含代码块的合法内容）\n        if (parsed.querySelector(\'*\') !== null) {\n            return true;\n        }\n\n        // (C) 无任何元素，但源串写了标签语法（< 紧跟字母 / 感叹号 / 斜杠）-> 实体转义未解码，整段退化为纯文本\n        if (/<[a-zA-Z!/]/.test(src)) {\n            return false;\n        }\n\n        return true;\n    }\n\n    function renderHtml(html, executeScripts, phase) {\n        currentPhase = phase || \'update\';\n        if (!root) {\n            reportError(phase, \'Render failed: root element not found\', \'\', {\n                source: \'sandbox\',\n                extra: {\n                    htmlLength: html ? html.length : 0,\n                    executeScripts: !!executeScripts\n                }\n            });\n            return;\n        }\n        try {\n            var temp = document.createElement(\'div\');\n            temp.innerHTML = toTrustedHTML(html || \'\');\n\n            bindInlineHandlers(temp);\n\n            var scripts = Array.prototype.slice.call(temp.querySelectorAll(\'script\'));\n            var useCanvasStabilizer = shouldStabilizeCanvas(scripts);\n            stabilizeCanvasContainers(temp, useCanvasStabilizer);\n            scripts.forEach(function(script) {\n                if (script.parentNode) {\n                    script.parentNode.removeChild(script);\n                }\n            });\n\n            // 渲染前校验（仅流式 update 帧）：内容未正确解析为 HTML（转义残留 / 字面量转义残片）时\n            // 跳过本帧、保留上一帧已渲染内容，避免「乱码 -> 纠正」造成的闪烁。\n            // finalize（executeScripts=true）为最终确定内容，不跳过。\n            if (!executeScripts &amp;&amp; !looksRenderable(html, temp)) {\n                return;\n            }\n\n            // 增量同步：原地 patch 差异节点、保留未变节点（尤其 canvas），\n            // 取代整体清空重建，消除流式最后一帧 -> finalize 的清空闪白。\n            // morph 出现异常时兜底退回全量重建，保证内容正确。\n            try {\n                morphChildren(root, temp);\n            } catch (morphError) {\n                console.warn(\'[WidgetSandbox] morph failed, fallback to full rebuild.\', morphError);\n                while (root.firstChild) {\n                    root.removeChild(root.firstChild);\n                }\n                while (temp.firstChild) {\n                    root.appendChild(temp.firstChild);\n                }\n            }\n\n            if (!executeScripts) {\n                setTimeout(reportHeight, 10);\n                return;\n            }\n\n            executeScriptsSequentially(scripts).finally(function() {\n                setTimeout(reportHeight, 50);\n            });\n        } catch (error) {\n            console.warn(\'[WidgetSandbox] render failed.\', error);\n            reportError(phase, \'Render failed: \' + error.message, error.stack || error.message, {\n                stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                extra: {\n                    htmlLength: html ? html.length : 0,\n                    executeScripts: !!executeScripts\n                }\n            });\n        }\n    }\n\n    // 消息监听\n    window.addEventListener(\'message\', function(e) {\n        var data = e.data;\n        if (!data || typeof data.type !== \'string\') return;\n\n        switch (data.type) {\n            case \'widget:host-ready\':\n                // Host 可能在消息监听挂载后才就绪，允许重试握手避免丢失初次 ready\n                parent.postMessage({ type: \'widget:ready\' }, \'*\');\n                reportHeight();\n                break;\n\n            case \'widget:update\':\n                renderHtml(data.html || \'\', false, \'update\');\n                break;\n\n            case \'widget:finalize\':\n                renderHtml(data.html || \'\', true, \'finalize\');\n                break;\n\n            case \'widget:theme\':\n                // 清理上次注入的 widget-theme vars，避免主题切换后旧值在 documentElement.style 残留\n                // （host 推送的 vars 列表可能因切换前后宿主样式表差异而不同）。仅清理本 handler 注入过的\n                // 键，不影响 sandbox srcDoc 自身在 :root 定义的默认调色板。\n                var prevInjected = window.__widgetInjectedThemeVars || [];\n                for (var pi = 0; pi < prevInjected.length; pi++) {\n                    document.documentElement.style.removeProperty(prevInjected[pi]);\n                }\n\n                var vars = data.vars || {};\n                var style = document.documentElement.style;\n                var nextInjected = [];\n                for (var k in vars) {\n                    if (vars.hasOwnProperty(k)) {\n                        style.setProperty(k, vars[k]);\n                        nextInjected.push(k);\n                    }\n                }\n                window.__widgetInjectedThemeVars = nextInjected;\n\n                if (data.isDark) {\n                    document.documentElement.classList.add(\'dark\');\n                } else {\n                    document.documentElement.classList.remove(\'dark\');\n                }\n                break;\n\n            case \'widget:capture\':\n                // 截图协议：将当前 DOM 渲染为 PNG 图片并回传给宿主\n                captureWidgetAsImage(data.requestId || \'\');\n                break;\n        }\n    });\n\n    // 截图结果回传 helper\n    function sendCaptureResult(requestId, dataUrl, width, height) {\n        parent.postMessage({ type: \'widget:capture-result\', requestId: requestId, dataUrl: dataUrl, width: width, height: height }, \'*\');\n    }\n    function sendCaptureError(requestId, error) {\n        parent.postMessage({ type: \'widget:capture-result\', requestId: requestId, error: error }, \'*\');\n    }\n\n    /**\n     * 将 widget DOM 捕获为 PNG 图片。\n     * 统一使用 foreignObject + SVG → Canvas 方案，canvas 元素内容会被内联为 img 保留。\n     */\n    function captureWidgetAsImage(requestId) {\n        if (!root) {\n            sendCaptureError(requestId, \'No root element\');\n            return;\n        }\n\n        try {\n            var rect = root.getBoundingClientRect();\n            var width = Math.max(Math.ceil(rect.width), 800);\n            var height = Math.ceil(rect.height);\n\n            if (height <= 0) {\n                sendCaptureError(requestId, \'Content has zero height\');\n                return;\n            }\n\n            if (height > CAPTURE_MAX_HEIGHT) {\n                sendCaptureError(requestId, \'Content too tall to capture (\' + height + \'px, limit \' + CAPTURE_MAX_HEIGHT + \'px)\');\n                return;\n            }\n\n            if (height > 8000) {\n                parent.postMessage({ type: \'widget:capture-progress\', requestId: requestId, message: \'large-content\', height: height }, \'*\');\n            }\n\n            captureViaForeignObject(requestId, width, height);\n        } catch (err) {\n            sendCaptureError(requestId, \'Capture failed: \' + (err.message || err));\n        }\n    }\n\n    /**\n     * 混合合成截图方案：\n     * 1. foreignObject 渲染 HTML/CSS 布局（canvas 替换为透明占位块）\n     * 2. 渲染完成后，将原始 canvas 内容叠加绘制到对应位置\n     * 这样既保留 HTML 标题/图例/文字，又保留 JS 图表的 canvas 绘制内容。\n     */\n    function captureViaForeignObject(requestId, width, height) {\n        // 收集所有样式\n        var cssText = \'\';\n        for (var si = 0; si < document.styleSheets.length; si++) {\n            try {\n                var sheet = document.styleSheets[si];\n                for (var ri = 0; ri < sheet.cssRules.length; ri++) {\n                    cssText += sheet.cssRules[ri].cssText + \'\\n\';\n                }\n            } catch (_e) {\n                // 跨域样式表无法读取规则，跳过\n            }\n        }\n\n        var rootStyle = document.documentElement.getAttribute(\'style\') || \'\';\n        var darkClass = document.documentElement.classList.contains(\'dark\') ? \' class=&quot;dark&quot;\' : \'\';\n\n        // 克隆 DOM\n        var clone = root.cloneNode(true);\n\n        // 记录原始 canvas 的位置和内容，同时在克隆中替换为透明占位块\n        var originalCanvases = root.querySelectorAll(\'canvas\');\n        var clonedCanvases = clone.querySelectorAll(\'canvas\');\n        var canvasOverlays = [];\n        var rootRect = root.getBoundingClientRect();\n\n        for (var ci = 0; ci < originalCanvases.length; ci++) {\n            try {\n                var originalCanvas = originalCanvases[ci];\n                var clonedCanvas = clonedCanvases[ci];\n                if (originalCanvas &amp;&amp; clonedCanvas &amp;&amp; clonedCanvas.parentNode) {\n                    // 记录位置和 data URL 用于后续叠加\n                    var cRect = originalCanvas.getBoundingClientRect();\n                    canvasOverlays.push({\n                        dataUrl: originalCanvas.toDataURL(\'image/png\'),\n                        x: cRect.left - rootRect.left,\n                        y: cRect.top - rootRect.top,\n                        w: cRect.width,\n                        h: cRect.height\n                    });\n\n                    // 替换为同尺寸的透明占位 div（保持布局）\n                    var placeholder = document.createElement(\'div\');\n                    placeholder.style.width = originalCanvas.offsetWidth + \'px\';\n                    placeholder.style.height = originalCanvas.offsetHeight + \'px\';\n                    placeholder.style.display = \'block\';\n                    clonedCanvas.parentNode.replaceChild(placeholder, clonedCanvas);\n                }\n            } catch (_canvasErr) {\n                // canvas 可能被 tainted，跳过\n            }\n        }\n\n        // 移除 script 标签\n        var scripts = clone.querySelectorAll(\'script\');\n        for (var ssi = 0; ssi < scripts.length; ssi++) {\n            scripts[ssi].parentNode.removeChild(scripts[ssi]);\n        }\n\n        var serializer = new XMLSerializer();\n        var htmlContent = serializer.serializeToString(clone);\n\n        // 构建 SVG foreignObject\n        var svgNs = \'http://www.w3.org/2000/svg\';\n        var xhtmlNs = \'http://www.w3.org/1999/xhtml\';\n        var svgData = \'<svg xmlns=&quot;\' + svgNs + \'&quot; width=&quot;\' + width + \'&quot; height=&quot;\' + height + \'&quot;>\' +\n            \'<foreignObject width=&quot;100%&quot; height=&quot;100%&quot;>\' +\n            \'<html xmlns=&quot;\' + xhtmlNs + \'&quot;\' + darkClass + \' style=&quot;\' + rootStyle.replace(/&quot;/g, \'&amp;quot;\') + \'&quot;>\' +\n            \'<head><style>\' + cssText.split(\'</style>\').join(\'</st\' + \'yle>\') + \'</style></head>\' +\n            \'<body style=&quot;margin:0;padding:0;overflow:hidden;&quot;>\' + htmlContent + \'</body>\' +\n            \'</html>\' +\n            \'</foreignObject>\' +\n            \'</svg>\';\n\n        var img = new Image();\n        img.onload = function() {\n            try {\n                var padding = 24;\n                var canvas = document.createElement(\'canvas\');\n                var scale = height > 8000 ? 1 : 2;\n                var totalWidth = width + padding * 2;\n                var totalHeight = height + padding * 2;\n                canvas.width = totalWidth * scale;\n                canvas.height = totalHeight * scale;\n                var ctx = canvas.getContext(\'2d\');\n                ctx.scale(scale, scale);\n\n                // 背景色跟随主题\n                var isDark = document.documentElement.classList.contains(\'dark\');\n                ctx.fillStyle = isDark ? \'#1e1e1e\' : \'#ffffff\';\n                ctx.fillRect(0, 0, totalWidth, totalHeight);\n\n                // 绘制 foreignObject 渲染的 HTML/CSS 内容（带 padding 偏移）\n                ctx.drawImage(img, padding, padding, width, height);\n\n                // 叠加绘制 canvas 元素内容到对应位置（加 padding 偏移）\n                if (canvasOverlays.length > 0) {\n                    var overlaysLoaded = 0;\n                    var totalOverlays = canvasOverlays.length;\n\n                    for (var oi = 0; oi < totalOverlays; oi++) {\n                        (function(overlay) {\n                            var overlayImg = new Image();\n                            overlayImg.onload = function() {\n                                ctx.drawImage(overlayImg, overlay.x + padding, overlay.y + padding, overlay.w, overlay.h);\n                                overlaysLoaded++;\n                                if (overlaysLoaded === totalOverlays) {\n                                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                                }\n                            };\n                            overlayImg.onerror = function() {\n                                overlaysLoaded++;\n                                if (overlaysLoaded === totalOverlays) {\n                                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                                }\n                            };\n                            overlayImg.src = overlay.dataUrl;\n                        })(canvasOverlays[oi]);\n                    }\n                } else {\n                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                }\n            } catch (canvasErr) {\n                sendCaptureError(requestId, \'Canvas render failed: \' + (canvasErr.message || canvasErr));\n            }\n        };\n        img.onerror = function() {\n            // foreignObject 渲染失败时，尝试纯 canvas 合成作为降级\n            var padding = 24;\n            var canvases = root.querySelectorAll(\'canvas\');\n            if (canvases.length > 0) {\n                try {\n                    var totalW = width + padding * 2;\n                    var totalH = height + padding * 2;\n                    var fallbackCanvas = document.createElement(\'canvas\');\n                    var scale = height > 8000 ? 1 : 2;\n                    fallbackCanvas.width = totalW * scale;\n                    fallbackCanvas.height = totalH * scale;\n                    var fCtx = fallbackCanvas.getContext(\'2d\');\n                    fCtx.scale(scale, scale);\n                    var isDark = document.documentElement.classList.contains(\'dark\');\n                    fCtx.fillStyle = isDark ? \'#1e1e1e\' : \'#ffffff\';\n                    fCtx.fillRect(0, 0, totalW, totalH);\n\n                    for (var fi = 0; fi < canvases.length; fi++) {\n                        var fc = canvases[fi];\n                        var fcRect = fc.getBoundingClientRect();\n                        fCtx.drawImage(fc, fcRect.left - rootRect.left + padding, fcRect.top - rootRect.top + padding, fcRect.width, fcRect.height);\n                    }\n\n                    sendCaptureResult(requestId, fallbackCanvas.toDataURL(\'image/png\'), totalW, totalH);\n                } catch (_fallbackErr) {\n                    sendCaptureError(requestId, \'SVG and canvas fallback both failed\');\n                }\n            } else {\n                sendCaptureError(requestId, \'SVG image load failed\');\n            }\n        };\n\n        var svgBase64 = btoa(unescape(encodeURIComponent(svgData)));\n        img.src = \'data:image/svg+xml;base64,\' + svgBase64;\n    }\n\n    // 通知宿主就绪\n    currentPhase = \'bootstrap\';\n    parent.postMessage({ type: \'widget:ready\' }, \'*\');\n})();';
var b=new Blob([s],{type:'text/javascript'});
var u=URL.createObjectURL(b);
var el=document.createElement('script');
var ttPolicy=null;


function initTrustedTypesPolicy(){
    if(ttPolicy||!window.trustedTypes||typeof window.trustedTypes.createPolicy!=='function') return;
    try{
        ttPolicy=window.trustedTypes.createPolicy('default',{
            createHTML:function(value){return value;},
            createScript:function(value){return value;},
            createScriptURL:function(value){return value;}
        });
    }catch(_error){
        ttPolicy=null;
    }
}

function toTrustedScriptURL(value){
    initTrustedTypesPolicy();
    if(ttPolicy&amp;&amp;typeof ttPolicy.createScriptURL==='function'){
        try{return ttPolicy.createScriptURL(value);}catch(_error){return value;}
    }
    return value;
}

function toTrustedScript(value){
    initTrustedTypesPolicy();
    if(ttPolicy&amp;&amp;typeof ttPolicy.createScript==='function'){
        try{return ttPolicy.createScript(value);}catch(_error){return value;}
    }
    return value;
}

function reportBootstrapFailure(reason, error){
    parent.postMessage({
        type:'widget:error',
        phase:'bootstrap',
        message:'Script load failed: '+(error&amp;&amp;error.message?error.message:'unknown'),
        detail:(error&amp;&amp;error.stack)||String(reason)||'',
        stack:error&amp;&amp;error.stack?String(error.stack):'',
        source:'bootstrap-script',
        runtime:{
            href:location.href,
            userAgent:navigator.userAgent,
            readyState:document.readyState,
            visibilityState:document.visibilityState,
            phase:'bootstrap',
            timestamp:new Date().toISOString()
        },
        extra:{
            bootstrapReason:String(reason||'')
        }
    },'*');
}

function tryEvalFallback(reason){
    try{
        // CSP 已允许 unsafe-eval，eval 不受 Trusted Types 约束
        (0,eval)(s);
        URL.revokeObjectURL(u);
        return true;
    }catch(error){
        reportBootstrapFailure(reason,error);
        URL.revokeObjectURL(u);
        return false;
    }
}

function tryInlineFallback(reason){
    try{
        var inline=document.createElement('script');
        var current=document.currentScript;
        var nonce=current&amp;&amp;typeof current.getAttribute==='function'?current.getAttribute('nonce'):'';
        if(nonce){
            inline.setAttribute('nonce',nonce);
        }
        inline.text=toTrustedScript(s);
        document.head.appendChild(inline);
        URL.revokeObjectURL(u);
        return true;
    }catch(error){
        // script.text 赋值被 Trusted Types 拦截时，使用 eval fallback
        return tryEvalFallback(reason);
    }
}

try {
    // 某些宿主启用 Trusted Types 时，script.src 需要 TrustedScriptURL
    el.src=toTrustedScriptURL(u);
} catch(assignError){
    tryInlineFallback(assignError&amp;&amp;assignError.message?assignError.message:assignError);
}

el.onload=function(){URL.revokeObjectURL(u)};
el.onerror=function(){
    tryInlineFallback('script-element-load-error');
};

try {
    document.head.appendChild(el);
} catch(appendError){
    tryInlineFallback(appendError&amp;&amp;appendError.message?appendError.message:appendError);
}</script>
</body>
</html>" title="AES_GCM_Encryption_Flow_in_DLMS" style="box-sizing: border-box; width: 786.391px; border: 0px; display: block; overflow: hidden; background: transparent; min-height: 0px; max-height: none; height: 554px;"></iframe>
<iframe class="_widgetFrame_pjcwq_189 " sandbox="allow-scripts" srcdoc="<!DOCTYPE html>
<html>
<head>
<meta charset=&quot;utf-8&quot;>
<meta http-equiv=&quot;Content-Security-Policy&quot; content=&quot;default-src 'none'; script-src 'unsafe-inline' 'unsafe-eval' blob: https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; style-src 'unsafe-inline'; img-src data: blob: https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; font-src https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; connect-src https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com&quot;>
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
html, body { width: 100%; height: auto; overflow: visible; }
/* iframe viewport 兜底：宿主 Math.ceil(reportHeight) 可能露出底部 0~1px，<html> 背景避免 dark 下白条。
   优先使用宿主对话面板同源 --cb-panel-bg-primary；缺失时回退 sandbox 内置 --color-background-primary。 */
html {
    background-color: var(--cb-panel-bg-primary, var(--color-background-primary, transparent));
}
/* 融入 chat UI 的滚动条样式：细、半透明、hover 时加深。
   注意：iframe srcDoc 是独立文档，宿主的 --cb-* CSS variables 不会继承进来，
   所以这里使用中性的半透明灰，保证明/暗主题下都协调。 */
html, body {
    scrollbar-width: thin;
    scrollbar-color: rgba(128, 128, 128, 0.3) transparent;
}
html::-webkit-scrollbar,
body::-webkit-scrollbar {
    width: 6px;
    height: 6px;
}
html::-webkit-scrollbar-track,
body::-webkit-scrollbar-track {
    background: transparent;
}
html::-webkit-scrollbar-thumb,
body::-webkit-scrollbar-thumb {
    background: rgba(128, 128, 128, 0.3);
    border-radius: 3px;
}
html::-webkit-scrollbar-thumb:hover,
body::-webkit-scrollbar-thumb:hover {
    background: rgba(128, 128, 128, 0.5);
}
html::-webkit-scrollbar-corner,
body::-webkit-scrollbar-corner {
    background: transparent;
}
body {
    font-family: var(--font-sans, system-ui, -apple-system, sans-serif);
    background: transparent;
    color: var(--color-text-primary, #1a1a1a);
    line-height: 1.5;
}
#root {
    width: 100%;
    min-height: 20px;
    /* 融入对话流：不再在 iframe 内部出现独立纵向滚动条。
       高度由外层通过 widget:resize 推断；绝对上限由宿主侧 clamp 保护。 */
    overflow: visible;
    overflow-x: hidden;
    /* widget_code 自管边距（prompt 已约束&quot;无顶部 padding&quot;），宿主不再塞 12px */
    padding: 0;
    /* 纯 SVG/透明 widget 未自绘背景时，由 #root 提供与对话面板一致的底色；
       widget 自带背景层按 CSS 层级覆盖此兜底。 */
    background-color: var(--cb-panel-bg-primary, var(--color-background-primary, transparent));
}
/* 确保 script 标签在流式阶段不被渲染为可见文本 */
script { display: none !important; }
/* SVG 箭头标记 */
svg defs marker#arrowhead { fill: var(--color-text-secondary, #666); }



AES-GCM 的流程清晰地展示了 DLMS 如何实现「一次操作，同时加密 + 认证」。其中几个关键设计点值得注意：

- **Nonce 构造**：`System Title (8B) + Invocation Counter (4B) = 12B`，确保每条消息的 IV 唯一
- **AAD (附加认证数据)**：包含安全控制字节、认证密钥、系统标题和调用计数器——这些数据不加密但被认证，防止篡改
- **认证标签**：12 字节（96 位），通过 GHASH 计算，提供完整性保证
- **防重放**：接收方检查 `Invocation Counter` 必须严格递增，相同或更低的值直接丢弃

------

### 三、密钥体系详解

DLMS 定义了丰富的密钥层次结构：

| 密钥类型             | 长度   | 用途                                   | 特点                                   |
| :------------------- | :----- | :------------------------------------- | :------------------------------------- |
| **Master Key (KEK)** | 16/32B | 密钥加密密钥，用于包装传输其他对称密钥 | 最高权限，永不直接用于数据加密         |
| **GUEK**             | 16/32B | 全局单播加密密钥                       | 保护点对点通信数据                     |
| **GBEK**             | 16/32B | 全局广播加密密钥                       | 保护广播消息（如固件升级通知）         |
| **GAK**              | 16/32B | 全局认证密钥                           | 用于 GMAC 认证和 AAD 构建              |
| **Dedicated Key**    | 16/32B | 专用密钥                               | 客户端在建立连接时提供，可每次随机生成 |
| **Ephemeral Key**    | 16/32B | 临时加密密钥                           | 通过 ECDH 密钥协商生成（Suite 1/2）    |

**密钥分发机制**有两种：

1. **密钥包装 (Key Wrapping)**：使用 KEK 通过 RFC 3394 AES Key Wrap 算法加密传输新密钥
2. **密钥协商 (Key Agreement)**：通过 ECDH 协议协商临时密钥（仅 Suite 1/2），提供前向保密

------

### 四、认证机制

DLMS 支持三个认证级别：

- **无认证**：公开客户端 (client_logical_address=16) 访问非敏感数据
- **LLS (Low Level Security)**：简单密码验证，明文传输密码
- **HLS (High Level Security)**：挑战-应答机制，密码不直接传输

HLS 是最重要的认证方式，下面是其 GMAC 变体的交互流程：

展示详情



<iframe class="_widgetFrame_pjcwq_189 " sandbox="allow-scripts" srcdoc="<!DOCTYPE html>
<html>
<head>
<meta charset=&quot;utf-8&quot;>
<meta http-equiv=&quot;Content-Security-Policy&quot; content=&quot;default-src 'none'; script-src 'unsafe-inline' 'unsafe-eval' blob: https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; style-src 'unsafe-inline'; img-src data: blob: https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; font-src https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com; connect-src https://cdnjs.cloudflare.com https://esm.sh https://cdn.jsdelivr.net https://unpkg.com&quot;>
<style>
*, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
html, body { width: 100%; height: auto; overflow: visible; }
/* iframe viewport 兜底：宿主 Math.ceil(reportHeight) 可能露出底部 0~1px，<html> 背景避免 dark 下白条。
   优先使用宿主对话面板同源 --cb-panel-bg-primary；缺失时回退 sandbox 内置 --color-background-primary。 */
html {
    background-color: var(--cb-panel-bg-primary, var(--color-background-primary, transparent));
}
/* 融入 chat UI 的滚动条样式：细、半透明、hover 时加深。
   注意：iframe srcDoc 是独立文档，宿主的 --cb-* CSS variables 不会继承进来，
   所以这里使用中性的半透明灰，保证明/暗主题下都协调。 */
html, body {
    scrollbar-width: thin;
    scrollbar-color: rgba(128, 128, 128, 0.3) transparent;
}
html::-webkit-scrollbar,
body::-webkit-scrollbar {
    width: 6px;
    height: 6px;
}
html::-webkit-scrollbar-track,
body::-webkit-scrollbar-track {
    background: transparent;
}
html::-webkit-scrollbar-thumb,
body::-webkit-scrollbar-thumb {
    background: rgba(128, 128, 128, 0.3);
    border-radius: 3px;
}
html::-webkit-scrollbar-thumb:hover,
body::-webkit-scrollbar-thumb:hover {
    background: rgba(128, 128, 128, 0.5);
}
html::-webkit-scrollbar-corner,
body::-webkit-scrollbar-corner {
    background: transparent;
}
body {
    font-family: var(--font-sans, system-ui, -apple-system, sans-serif);
    background: transparent;
    color: var(--color-text-primary, #1a1a1a);
    line-height: 1.5;
}
#root {
    width: 100%;
    min-height: 20px;
    /* 融入对话流：不再在 iframe 内部出现独立纵向滚动条。
       高度由外层通过 widget:resize 推断；绝对上限由宿主侧 clamp 保护。 */
    overflow: visible;
    overflow-x: hidden;
    /* widget_code 自管边距（prompt 已约束&quot;无顶部 padding&quot;），宿主不再塞 12px */
    padding: 0;
    /* 纯 SVG/透明 widget 未自绘背景时，由 #root 提供与对话面板一致的底色；
       widget 自带背景层按 CSS 层级覆盖此兜底。 */
    background-color: var(--cb-panel-bg-primary, var(--color-background-primary, transparent));
}
/* 确保 script 标签在流式阶段不被渲染为可见文本 */
script { display: none !important; }
/* SVG 箭头标记 */
svg defs marker#arrowhead { fill: var(--color-text-secondary, #666); }

/* 文本类 */
.t { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 14px; fill: var(--color-text-primary, currentColor); }
.ts { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 12px; fill: var(--color-text-secondary, currentColor); }
.th { font-family: var(--font-sans, system-ui, -apple-system, sans-serif); font-size: 14px; font-weight: 500; fill: var(--color-text-primary, currentColor); }

/* 容器类 */
.box { fill: var(--color-background-secondary, #f5f5f5); stroke: var(--color-border-tertiary, #e0e0e0); stroke-width: 1; rx: 6; }
.node { cursor: pointer; }
.node:hover .box { fill: var(--color-background-tertiary, #ebebeb); }

/* 线条类 */
.arr { stroke: var(--color-text-secondary, #666); stroke-width: 1.5; fill: none; marker-end: url(#arrowhead); }
.leader { stroke: var(--color-border-tertiary, #ccc); stroke-width: 0.5; stroke-dasharray: 4 2; fill: none; }

/* 色板类 — 浅色默认值，暗色通过 .dark 祖先类覆盖 */
.c-purple { --node-bg: #EEEDFE; --node-border: #7F77DD; --node-text: #26215C; --node-text-sub: #534AB7; }
.c-teal   { --node-bg: #E1F5EE; --node-border: #1D9E75; --node-text: #04342C; --node-text-sub: #0F6E56; }
.c-coral  { --node-bg: #FAECE7; --node-border: #D85A30; --node-text: #4A1B0C; --node-text-sub: #9B3318; }
.c-pink   { --node-bg: #FBEAF0; --node-border: #D4537E; --node-text: #4B1528; --node-text-sub: #9C2D56; }
.c-gray   { --node-bg: #F1EFE8; --node-border: #888780; --node-text: #2C2C2A; --node-text-sub: #5C5C5A; }
.c-blue   { --node-bg: #E6F1FB; --node-border: #378ADD; --node-text: #042C53; --node-text-sub: #1B5C99; }
.c-green  { --node-bg: #EAF3DE; --node-border: #639922; --node-text: #173404; --node-text-sub: #3A6B10; }
.c-amber  { --node-bg: #FAEEDA; --node-border: #BA7517; --node-text: #412402; --node-text-sub: #7A4A10; }
.c-red    { --node-bg: #FCEBEB; --node-border: #E24B4A; --node-text: #501313; --node-text-sub: #9B2222; }

/* 暗色模式：覆盖色板变量（深背景 + 浅文字）
 * 双路触发：.dark 类（widget:theme 消息注入）+ prefers-color-scheme 媒体查询（不依赖消息时序）。
 * 媒体查询确保首次渲染时主题已就绪；.dark 类确保宿主显式切换主题时能覆盖系统偏好。 */
.dark .c-purple { --node-bg: #26215C; --node-border: #7F77DD; --node-text: #EEEDFE; --node-text-sub: #B8B4F5; }
.dark .c-teal   { --node-bg: #04342C; --node-border: #1D9E75; --node-text: #E1F5EE; --node-text-sub: #7DDDC0; }
.dark .c-coral  { --node-bg: #4A1B0C; --node-border: #D85A30; --node-text: #FAECE7; --node-text-sub: #F0A080; }
.dark .c-pink   { --node-bg: #4B1528; --node-border: #D4537E; --node-text: #FBEAF0; --node-text-sub: #F0A0C0; }
.dark .c-gray   { --node-bg: #2C2C2A; --node-border: #888780; --node-text: #F1EFE8; --node-text-sub: #B8B6B0; }
.dark .c-blue   { --node-bg: #042C53; --node-border: #378ADD; --node-text: #E6F1FB; --node-text-sub: #90C4F0; }
.dark .c-green  { --node-bg: #173404; --node-border: #639922; --node-text: #EAF3DE; --node-text-sub: #A0D060; }
.dark .c-amber  { --node-bg: #412402; --node-border: #BA7517; --node-text: #FAEEDA; --node-text-sub: #E0A860; }
.dark .c-red    { --node-bg: #501313; --node-border: #E24B4A; --node-text: #FCEBEB; --node-text-sub: #F09090; }

@media (prefers-color-scheme: dark) {
    .c-purple { --node-bg: #26215C; --node-border: #7F77DD; --node-text: #EEEDFE; --node-text-sub: #B8B4F5; }
    .c-teal   { --node-bg: #04342C; --node-border: #1D9E75; --node-text: #E1F5EE; --node-text-sub: #7DDDC0; }
    .c-coral  { --node-bg: #4A1B0C; --node-border: #D85A30; --node-text: #FAECE7; --node-text-sub: #F0A080; }
    .c-pink   { --node-bg: #4B1528; --node-border: #D4537E; --node-text: #FBEAF0; --node-text-sub: #F0A0C0; }
    .c-gray   { --node-bg: #2C2C2A; --node-border: #888780; --node-text: #F1EFE8; --node-text-sub: #B8B6B0; }
    .c-blue   { --node-bg: #042C53; --node-border: #378ADD; --node-text: #E6F1FB; --node-text-sub: #90C4F0; }
    .c-green  { --node-bg: #173404; --node-border: #639922; --node-text: #EAF3DE; --node-text-sub: #A0D060; }
    .c-amber  { --node-bg: #412402; --node-border: #BA7517; --node-text: #FAEEDA; --node-text-sub: #E0A860; }
    .c-red    { --node-bg: #501313; --node-border: #E24B4A; --node-text: #FCEBEB; --node-text-sub: #F09090; }
}

/* 语义类（.box / .t / .th / .ts）消费色板变量 */
.c-purple .box, .c-teal .box, .c-coral .box, .c-pink .box, .c-gray .box,
.c-blue .box, .c-green .box, .c-amber .box, .c-red .box {
    fill: var(--node-bg); stroke: var(--node-border);
}
.c-purple .t, .c-teal .t, .c-coral .t, .c-pink .t, .c-gray .t,
.c-blue .t, .c-green .t, .c-amber .t, .c-red .t,
.c-purple .th, .c-teal .th, .c-coral .th, .c-pink .th, .c-gray .th,
.c-blue .th, .c-green .th, .c-amber .th, .c-red .th {
    fill: var(--node-text);
}
.c-purple .ts, .c-teal .ts, .c-coral .ts, .c-pink .ts, .c-gray .ts,
.c-blue .ts, .c-green .ts, .c-amber .ts, .c-red .ts {
    fill: var(--node-text-sub);
}

/* 兜底：模型直接输出 SVG 原生元素（无语义类）时自动应用色板变量。
 * 仅覆盖封闭形状（rect/circle/ellipse）；path 可能是箭头/线条，不统一覆盖。
 * SVG presentation attribute（fill=&quot;...&quot;）优先级低于 CSS，无需 !important。 */
.c-purple rect, .c-teal rect, .c-coral rect, .c-pink rect, .c-gray rect,
.c-blue rect, .c-green rect, .c-amber rect, .c-red rect,
.c-purple circle, .c-teal circle, .c-coral circle, .c-pink circle, .c-gray circle,
.c-blue circle, .c-green circle, .c-amber circle, .c-red circle,
.c-purple ellipse, .c-teal ellipse, .c-coral ellipse, .c-pink ellipse, .c-gray ellipse,
.c-blue ellipse, .c-green ellipse, .c-amber ellipse, .c-red ellipse {
    fill: var(--node-bg); stroke: var(--node-border);
}
/* text 的 fill 属性同为 presentation attribute，CSS 可直接覆盖，无需 !important */
.c-purple text, .c-teal text, .c-coral text, .c-pink text, .c-gray text,
.c-blue text, .c-green text, .c-amber text, .c-red text {
    fill: var(--node-text);
}


:root {
    /* 背景色 */
    --color-background-primary: #ffffff;
    --color-background-secondary: #f5f5f5;
    --color-background-tertiary: #ebebeb;

    /* 文本色 */
    --color-text-primary: #1a1a1a;
    --color-text-secondary: #666666;
    --color-text-tertiary: #999999;
    
    /* 边框色 */
    --color-border-primary: rgba(0, 0, 0, 0.4);
    --color-border-secondary: rgba(0, 0, 0, 0.3);
    --color-border-tertiary: rgba(0, 0, 0, 0.15);
    
    /* 字体 */
    --font-sans: system-ui, -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
    --font-serif: Georgia, 'Times New Roman', serif;
    --font-mono: 'SF Mono', Monaco, 'Cascadia Code', 'Roboto Mono', Consolas, monospace;
    
    /* 圆角 */
    --border-radius-sm: 4px;
    --border-radius-md: 8px;
    --border-radius-lg: 12px;
    --border-radius-xl: 16px;
}

.dark {
    --color-background-primary: #1a1a1a;
    --color-background-secondary: #2a2a2a;
    --color-background-tertiary: #333333;

    --color-text-primary: #e0e0e0;
    --color-text-secondary: #a0a0a0;
    --color-text-tertiary: #707070;
    
    --color-border-primary: rgba(255, 255, 255, 0.4);
    --color-border-secondary: rgba(255, 255, 255, 0.3);
    --color-border-tertiary: rgba(255, 255, 255, 0.15);
}

</style>
</head>
<body>
<div id=&quot;root&quot;></div>
<script>
var s='(function() {\n    \'use strict\';\n    var root = document.getElementById(\'root\');\n    var pendingResize = false;\n    var lastHeight = 0;\n    var currentPhase = \'bootstrap\';\n    var MIN_WIDGET_HEIGHT = 20;\n    var MAX_WIDGET_HEIGHT = 2000;\n    var CANVAS_FALLBACK_HEIGHT_PX = 320;\n    var CANVAS_MIN_HEIGHT_PX = 120;\n    var CAPTURE_MAX_HEIGHT = 16384;\n    var INLINE_PX_HEIGHT_RE = /^(\\d+(?:\\.\\d+)?)px$/;\n    var trustedTypesPolicy = null;\n\n    function initTrustedTypesPolicy() {\n        if (trustedTypesPolicy || typeof window === \'undefined\' || !window.trustedTypes || typeof window.trustedTypes.createPolicy !== \'function\') {\n            return;\n        }\n        try {\n            trustedTypesPolicy = window.trustedTypes.createPolicy(\'default\', {\n                createHTML: function(value) { return value; },\n                createScript: function(value) { return value; },\n                createScriptURL: function(value) { return value; }\n            });\n        } catch (_error) {\n            trustedTypesPolicy = null;\n        }\n    }\n\n    function toTrustedScript(code) {\n        initTrustedTypesPolicy();\n        if (trustedTypesPolicy &amp;&amp; typeof trustedTypesPolicy.createScript === \'function\') {\n            try {\n                return trustedTypesPolicy.createScript(code);\n            } catch (_error) {\n                return code;\n            }\n        }\n        return code;\n    }\n\n    function toTrustedHTML(html) {\n        initTrustedTypesPolicy();\n        if (trustedTypesPolicy &amp;&amp; typeof trustedTypesPolicy.createHTML === \'function\') {\n            try {\n                return trustedTypesPolicy.createHTML(html);\n            } catch (_error) {\n                return html;\n            }\n        }\n        return html;\n    }\n\n    function getRuntimeContext() {\n        return {\n            href: location.href,\n            userAgent: navigator.userAgent,\n            readyState: document.readyState,\n            visibilityState: document.visibilityState,\n            phase: currentPhase,\n            timestamp: new Date().toISOString()\n        };\n    }\n\n    function stringifyUnknown(value) {\n        if (value == null) {\n            return \'\';\n        }\n        if (typeof value === \'string\') {\n            return value;\n        }\n        if (typeof value === \'number\' || typeof value === \'boolean\') {\n            return String(value);\n        }\n        try {\n            return JSON.stringify(value);\n        } catch (_unusedError) {\n            return Object.prototype.toString.call(value);\n        }\n    }\n\n    function reportError(phase, message, detail, meta) {\n        var payload = {\n            type: \'widget:error\',\n            phase: phase || currentPhase,\n            message: message || \'Unknown error\',\n            detail: detail || \'\',\n            stack: meta &amp;&amp; meta.stack ? String(meta.stack) : \'\',\n            source: meta &amp;&amp; meta.source ? String(meta.source) : \'\',\n            line: meta &amp;&amp; typeof meta.line === \'number\' ? meta.line : undefined,\n            column: meta &amp;&amp; typeof meta.column === \'number\' ? meta.column : undefined,\n            runtime: getRuntimeContext(),\n            extra: meta &amp;&amp; meta.extra ? meta.extra : undefined\n        };\n        parent.postMessage(payload, \'*\');\n    }\n\n    // 全局错误捕获\n    window.onerror = function(message, source, lineno, colno, error) {\n        var fallbackDetail = source ? (source + \':\' + lineno + \':\' + colno) : \'\';\n        var stack = error &amp;&amp; error.stack ? String(error.stack) : \'\';\n        var detail = stack || fallbackDetail;\n        reportError(currentPhase, String(message || \'window.onerror\'), detail, {\n            stack: stack,\n            source: source,\n            line: typeof lineno === \'number\' ? lineno : undefined,\n            column: typeof colno === \'number\' ? colno : undefined,\n            extra: {\n                errorName: error &amp;&amp; error.name ? error.name : \'\'\n            }\n        });\n        return true;\n    };\n\n    // Promise 未捕获错误\n    window.onunhandledrejection = function(event) {\n        var reason = event &amp;&amp; event.reason;\n        var isError = reason instanceof Error;\n        var message = isError ? reason.message : stringifyUnknown(reason);\n        var stack = isError &amp;&amp; reason.stack ? String(reason.stack) : \'\';\n        var detail = stack || stringifyUnknown(reason);\n\n        reportError(currentPhase, \'Unhandled Promise rejection: \' + (message || \'unknown\'), detail, {\n            stack: stack,\n            extra: {\n                reasonType: Object.prototype.toString.call(reason)\n            }\n        });\n    };\n\n    // 全局函数：发送消息到对话\n    window.sendPrompt = function(text) {\n        if (typeof text !== \'string\') text = String(text || \'\');\n        parent.postMessage({\n            type: \'widget:sendMessage\',\n            text: text.slice(0, 500)\n        }, \'*\');\n    };\n\n    // 全局函数：打开外部链接\n    window.openLink = function(url) {\n        if (typeof url !== \'string\') return;\n        if (/^(javascript|data):/i.test(url)) return;\n        parent.postMessage({ type: \'widget:link\', href: url }, \'*\');\n    };\n\n    // 拦截链接点击\n    document.addEventListener(\'click\', function(e) {\n        var a = e.target.closest ? e.target.closest(\'a[href]\') : null;\n        if (a) {\n            e.preventDefault();\n            e.stopPropagation();\n            openLink(a.href);\n        }\n    }, true);\n\n    // 高度自适应（ResizeObserver + rAF 节流）\n    function reportHeight() {\n        if (!root) {\n            return;\n        }\n        var h = Math.ceil(root.getBoundingClientRect().height);\n        h = Math.min(Math.max(h, MIN_WIDGET_HEIGHT), MAX_WIDGET_HEIGHT);\n        if (h !== lastHeight &amp;&amp; h > 0) {\n            lastHeight = h;\n            parent.postMessage({ type: \'widget:resize\', height: h }, \'*\');\n        }\n    }\n\n    var ro = new ResizeObserver(function() {\n        if (pendingResize) return;\n        pendingResize = true;\n        requestAnimationFrame(function() {\n            pendingResize = false;\n            reportHeight();\n        });\n    });\n    if (root) {\n        ro.observe(root);\n    }\n\n    function shouldStabilizeCanvas(scripts) {\n        for (var i = 0; i < scripts.length; i++) {\n            var script = scripts[i];\n            var src = (script.getAttribute(\'src\') || \'\').toLowerCase();\n            var code = (script.textContent || \'\').toLowerCase();\n            if (src.indexOf(\'chart\') >= 0 || src.indexOf(\'chart.js\') >= 0 || code.indexOf(\'new chart(\') >= 0) {\n                return true;\n            }\n        }\n        return false;\n    }\n\n    /**\n     * 高度来源优先级：父容器 inline px -> canvas height 属性 -> 实测高度 -> 兜底高度，\n     * 最终 clamp 到 [CANVAS_MIN_HEIGHT_PX, MAX_WIDGET_HEIGHT]。\n     */\n    function stabilizeCanvasContainers(scope, enable) {\n        if (!enable) {\n            return;\n        }\n\n        var canvases = scope.querySelectorAll(\'canvas\');\n        for (var i = 0; i < canvases.length; i++) {\n            var canvas = canvases[i];\n            if (!canvas || canvas.getAttribute(\'data-widget-canvas-stable\') === \'1\') {\n                continue;\n            }\n\n            var parent = canvas.parentElement;\n            if (!parent) {\n                continue;\n            }\n\n            var parentInlineHeight = NaN;\n            if (parent.style &amp;&amp; typeof parent.style.height === \'string\') {\n                var parentHeightStr = parent.style.height.trim();\n                var parentMatch = INLINE_PX_HEIGHT_RE.exec(parentHeightStr);\n                if (parentMatch) {\n                    parentInlineHeight = parseFloat(parentMatch[1]);\n                }\n            }\n            var heightAttr = parseInt(canvas.getAttribute(\'height\') || \'\', 10);\n            var measured = Math.ceil(canvas.getBoundingClientRect ? canvas.getBoundingClientRect().height : 0);\n            var stableHeight = Number.isFinite(parentInlineHeight) &amp;&amp; parentInlineHeight > 0 ? parentInlineHeight :\n                Number.isFinite(heightAttr) &amp;&amp; heightAttr > 0 ? heightAttr :\n                measured > 0 ? measured :\n                CANVAS_FALLBACK_HEIGHT_PX;\n            stableHeight = Math.min(Math.max(stableHeight, CANVAS_MIN_HEIGHT_PX), MAX_WIDGET_HEIGHT);\n\n            var wrapper = document.createElement(\'div\');\n            wrapper.className = \'widget-canvas-shell\';\n            wrapper.style.position = \'relative\';\n            wrapper.style.width = \'100%\';\n            wrapper.style.height = stableHeight + \'px\';\n            wrapper.style.minHeight = stableHeight + \'px\';\n            wrapper.style.maxHeight = stableHeight + \'px\';\n            wrapper.style.overflow = \'hidden\';\n\n            parent.insertBefore(wrapper, canvas);\n            wrapper.appendChild(canvas);\n\n            canvas.style.width = \'100%\';\n            canvas.style.height = \'100%\';\n            canvas.style.display = \'block\';\n            canvas.setAttribute(\'data-widget-canvas-stable\', \'1\');\n        }\n    }\n\n    function bindInlineHandlers(scope) {\n        var elements = scope.querySelectorAll(\'*\');\n        for (var i = 0; i < elements.length; i++) {\n            var element = elements[i];\n            var attrs = element.getAttributeNames ? element.getAttributeNames() : [];\n            for (var j = 0; j < attrs.length; j++) {\n                var attr = attrs[j];\n                if (!/^on/i.test(attr)) continue;\n                var eventName = attr.slice(2).toLowerCase();\n                var code = element.getAttribute(attr);\n                if (!eventName || !code) continue;\n\n                element.removeAttribute(attr);\n                element.addEventListener(eventName, function(event) {\n                    var target = event.currentTarget;\n                    var handlerCode = target &amp;&amp; target.__widgetInlineHandlerCode &amp;&amp; target.__widgetInlineHandlerCode[event.type];\n                    if (!handlerCode) return;\n                    try {\n                        var handler = new Function(\'event\', toTrustedScript(handlerCode));\n                        handler.call(target, event);\n                    } catch (error) {\n                        console.warn(\'[WidgetSandbox] inline handler execution failed.\', error);\n                        reportError(\'script\', \'Inline handler failed: \' + error.message, error.stack || error.message, {\n                            stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                            extra: {\n                                eventType: event &amp;&amp; event.type ? event.type : \'\'\n                            }\n                        });\n                    }\n                });\n\n                if (!element.__widgetInlineHandlerCode) {\n                    element.__widgetInlineHandlerCode = {};\n                }\n                element.__widgetInlineHandlerCode[eventName] = code;\n            }\n        }\n    }\n\n    async function executeScriptsSequentially(scripts) {\n        currentPhase = \'script\';\n        for (var i = 0; i < scripts.length; i++) {\n            var script = scripts[i];\n            var src = script.getAttribute(\'src\');\n            var code = script.textContent || \'\';\n\n            try {\n                if (src) {\n                    var response = await fetch(src);\n                    if (!response.ok) throw new Error(\'fetch failed: \' + response.status);\n                    var externalCode = await response.text();\n                    (0, eval)(toTrustedScript(externalCode));\n                } else if (code.trim()) {\n                    (0, eval)(toTrustedScript(code));\n                }\n            } catch (error) {\n                console.warn(\'[WidgetSandbox] script execution failed.\', { src: src, error: error });\n                reportError(\'script\', \'Script execution failed\' + (src ? \' (\' + src + \')\' : \'\'), error.stack || error.message, {\n                    stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                    source: src || \'\',\n                    extra: {\n                        scriptType: src ? \'external\' : \'inline\'\n                    }\n                });\n            }\n        }\n    }\n\n    // 同步元素属性：仅写入有变化的属性、移除新内容里不存在的属性，\n    // 避免无谓的 setAttribute 触发重排。\n    function morphElementAttributes(fromEl, toEl) {\n        var toAttrs = toEl.attributes;\n        for (var i = 0; i < toAttrs.length; i++) {\n            var attr = toAttrs[i];\n            if (fromEl.getAttribute(attr.name) !== attr.value) {\n                fromEl.setAttribute(attr.name, attr.value);\n            }\n        }\n        var fromAttrs = fromEl.attributes;\n        for (var j = fromAttrs.length - 1; j >= 0; j--) {\n            var name = fromAttrs[j].name;\n            if (!toEl.hasAttribute(name)) {\n                fromEl.removeAttribute(name);\n            }\n        }\n    }\n\n    // 增量同步单个节点：类型/标签相同则原地更新，不同则整体替换。\n    function morphNode(fromNode, toNode) {\n        if (fromNode.nodeType !== toNode.nodeType || fromNode.nodeName !== toNode.nodeName) {\n            fromNode.parentNode.replaceChild(toNode, fromNode);\n            return;\n        }\n        // 文本 / 注释：内容变化时才改 nodeValue，不重建节点\n        if (fromNode.nodeType === 3 || fromNode.nodeType === 8) {\n            if (fromNode.nodeValue !== toNode.nodeValue) {\n                fromNode.nodeValue = toNode.nodeValue;\n            }\n            return;\n        }\n        if (fromNode.nodeType !== 1) {\n            return;\n        }\n        // 保留已存在的 canvas 节点（含已绘制内容与稳定高度），避免清空画布导致闪烁/图表丢失\n        if (fromNode.nodeName === \'CANVAS\') {\n            return;\n        }\n        morphElementAttributes(fromNode, toNode);\n        morphChildren(fromNode, toNode);\n    }\n\n    // 增量同步子节点列表：按位置逐个 morph，保留未变节点，仅删除/新增差异节点。\n    // 取代「整体清空再重建」，消除 finalize/update 切换时的清空闪白。\n    function morphChildren(fromEl, toEl) {\n        var toNodes = Array.prototype.slice.call(toEl.childNodes);\n        for (var i = 0; i < toNodes.length; i++) {\n            var toNode = toNodes[i];\n            var fromNode = fromEl.childNodes[i];\n            if (!fromNode) {\n                fromEl.appendChild(toNode);\n            } else {\n                morphNode(fromNode, toNode);\n            }\n        }\n        while (fromEl.childNodes.length > toNodes.length) {\n            fromEl.removeChild(fromEl.lastChild);\n        }\n    }\n\n    // 渲染前校验：判断 temp（已解析的离屏内容）是否为「可正常渲染的有效 HTML」。\n    // 流式中间帧偶发：上游内容尚未解码（实体转义残留如 &amp;lt;div&amp;gt;，或换行/制表符的字面量转义残片），\n    // 此时 innerHTML 会把整段当作纯文本，morph 进画面后出现乱码、随后被正确帧纠正，造成闪烁。\n    // 返回 false 时调用方跳过本帧、保留上一帧已渲染内容。\n    // 注意：本函数被嵌入模板字符串注入到 iframe，正则中的反斜杠需在源码里双写以保留到注入脚本。\n    function looksRenderable(sourceHtml, parsed) {\n        var src = sourceHtml || \'\';\n        if (!src) {\n            return false;\n        }\n\n        // (A) 未解码 JSON 转义残留检测 —— 必须在 hasElement 判断之前。\n        // 上游 JSON 字符串尚未解码时，HTML 里会残留「反斜杠+引号」(属性引号未解码)\n        // 或「反斜杠+ n/t/r」(字面换行、制表)。正常 HTML 用裸引号和真实空白，绝不出现这些组合。\n        // 这类脏帧仍能解析出真实元素节点，若直接 morph 进画面，残留字符会被当作可见文本画出来，\n        // 下一帧解码后再纠正 -> 表现为闪烁。出现 >=2 处即判定整帧不可渲染，保留上一帧。\n        // 用 charCode 遍历而非正则：本函数注入到 iframe 模板字符串，正则里的反斜杠转义极易写错。\n        var BACKSLASH_CODE = 92;\n        var undecodedHits = 0;\n        for (var ci = 0; ci < src.length - 1; ci++) {\n            if (src.charCodeAt(ci) === BACKSLASH_CODE) {\n                var nextChar = src.charAt(ci + 1);\n                if (nextChar === \'&quot;\' || nextChar === \'n\' || nextChar === \'t\' || nextChar === \'r\') {\n                    undecodedHits++;\n                    if (undecodedHits >= 2) {\n                        return false;\n                    }\n                }\n            }\n        }\n\n        // (B) 已解析出真实元素结构 -> 视为可渲染（不误伤含代码块的合法内容）\n        if (parsed.querySelector(\'*\') !== null) {\n            return true;\n        }\n\n        // (C) 无任何元素，但源串写了标签语法（< 紧跟字母 / 感叹号 / 斜杠）-> 实体转义未解码，整段退化为纯文本\n        if (/<[a-zA-Z!/]/.test(src)) {\n            return false;\n        }\n\n        return true;\n    }\n\n    function renderHtml(html, executeScripts, phase) {\n        currentPhase = phase || \'update\';\n        if (!root) {\n            reportError(phase, \'Render failed: root element not found\', \'\', {\n                source: \'sandbox\',\n                extra: {\n                    htmlLength: html ? html.length : 0,\n                    executeScripts: !!executeScripts\n                }\n            });\n            return;\n        }\n        try {\n            var temp = document.createElement(\'div\');\n            temp.innerHTML = toTrustedHTML(html || \'\');\n\n            bindInlineHandlers(temp);\n\n            var scripts = Array.prototype.slice.call(temp.querySelectorAll(\'script\'));\n            var useCanvasStabilizer = shouldStabilizeCanvas(scripts);\n            stabilizeCanvasContainers(temp, useCanvasStabilizer);\n            scripts.forEach(function(script) {\n                if (script.parentNode) {\n                    script.parentNode.removeChild(script);\n                }\n            });\n\n            // 渲染前校验（仅流式 update 帧）：内容未正确解析为 HTML（转义残留 / 字面量转义残片）时\n            // 跳过本帧、保留上一帧已渲染内容，避免「乱码 -> 纠正」造成的闪烁。\n            // finalize（executeScripts=true）为最终确定内容，不跳过。\n            if (!executeScripts &amp;&amp; !looksRenderable(html, temp)) {\n                return;\n            }\n\n            // 增量同步：原地 patch 差异节点、保留未变节点（尤其 canvas），\n            // 取代整体清空重建，消除流式最后一帧 -> finalize 的清空闪白。\n            // morph 出现异常时兜底退回全量重建，保证内容正确。\n            try {\n                morphChildren(root, temp);\n            } catch (morphError) {\n                console.warn(\'[WidgetSandbox] morph failed, fallback to full rebuild.\', morphError);\n                while (root.firstChild) {\n                    root.removeChild(root.firstChild);\n                }\n                while (temp.firstChild) {\n                    root.appendChild(temp.firstChild);\n                }\n            }\n\n            if (!executeScripts) {\n                setTimeout(reportHeight, 10);\n                return;\n            }\n\n            executeScriptsSequentially(scripts).finally(function() {\n                setTimeout(reportHeight, 50);\n            });\n        } catch (error) {\n            console.warn(\'[WidgetSandbox] render failed.\', error);\n            reportError(phase, \'Render failed: \' + error.message, error.stack || error.message, {\n                stack: error &amp;&amp; error.stack ? error.stack : \'\',\n                extra: {\n                    htmlLength: html ? html.length : 0,\n                    executeScripts: !!executeScripts\n                }\n            });\n        }\n    }\n\n    // 消息监听\n    window.addEventListener(\'message\', function(e) {\n        var data = e.data;\n        if (!data || typeof data.type !== \'string\') return;\n\n        switch (data.type) {\n            case \'widget:host-ready\':\n                // Host 可能在消息监听挂载后才就绪，允许重试握手避免丢失初次 ready\n                parent.postMessage({ type: \'widget:ready\' }, \'*\');\n                reportHeight();\n                break;\n\n            case \'widget:update\':\n                renderHtml(data.html || \'\', false, \'update\');\n                break;\n\n            case \'widget:finalize\':\n                renderHtml(data.html || \'\', true, \'finalize\');\n                break;\n\n            case \'widget:theme\':\n                // 清理上次注入的 widget-theme vars，避免主题切换后旧值在 documentElement.style 残留\n                // （host 推送的 vars 列表可能因切换前后宿主样式表差异而不同）。仅清理本 handler 注入过的\n                // 键，不影响 sandbox srcDoc 自身在 :root 定义的默认调色板。\n                var prevInjected = window.__widgetInjectedThemeVars || [];\n                for (var pi = 0; pi < prevInjected.length; pi++) {\n                    document.documentElement.style.removeProperty(prevInjected[pi]);\n                }\n\n                var vars = data.vars || {};\n                var style = document.documentElement.style;\n                var nextInjected = [];\n                for (var k in vars) {\n                    if (vars.hasOwnProperty(k)) {\n                        style.setProperty(k, vars[k]);\n                        nextInjected.push(k);\n                    }\n                }\n                window.__widgetInjectedThemeVars = nextInjected;\n\n                if (data.isDark) {\n                    document.documentElement.classList.add(\'dark\');\n                } else {\n                    document.documentElement.classList.remove(\'dark\');\n                }\n                break;\n\n            case \'widget:capture\':\n                // 截图协议：将当前 DOM 渲染为 PNG 图片并回传给宿主\n                captureWidgetAsImage(data.requestId || \'\');\n                break;\n        }\n    });\n\n    // 截图结果回传 helper\n    function sendCaptureResult(requestId, dataUrl, width, height) {\n        parent.postMessage({ type: \'widget:capture-result\', requestId: requestId, dataUrl: dataUrl, width: width, height: height }, \'*\');\n    }\n    function sendCaptureError(requestId, error) {\n        parent.postMessage({ type: \'widget:capture-result\', requestId: requestId, error: error }, \'*\');\n    }\n\n    /**\n     * 将 widget DOM 捕获为 PNG 图片。\n     * 统一使用 foreignObject + SVG → Canvas 方案，canvas 元素内容会被内联为 img 保留。\n     */\n    function captureWidgetAsImage(requestId) {\n        if (!root) {\n            sendCaptureError(requestId, \'No root element\');\n            return;\n        }\n\n        try {\n            var rect = root.getBoundingClientRect();\n            var width = Math.max(Math.ceil(rect.width), 800);\n            var height = Math.ceil(rect.height);\n\n            if (height <= 0) {\n                sendCaptureError(requestId, \'Content has zero height\');\n                return;\n            }\n\n            if (height > CAPTURE_MAX_HEIGHT) {\n                sendCaptureError(requestId, \'Content too tall to capture (\' + height + \'px, limit \' + CAPTURE_MAX_HEIGHT + \'px)\');\n                return;\n            }\n\n            if (height > 8000) {\n                parent.postMessage({ type: \'widget:capture-progress\', requestId: requestId, message: \'large-content\', height: height }, \'*\');\n            }\n\n            captureViaForeignObject(requestId, width, height);\n        } catch (err) {\n            sendCaptureError(requestId, \'Capture failed: \' + (err.message || err));\n        }\n    }\n\n    /**\n     * 混合合成截图方案：\n     * 1. foreignObject 渲染 HTML/CSS 布局（canvas 替换为透明占位块）\n     * 2. 渲染完成后，将原始 canvas 内容叠加绘制到对应位置\n     * 这样既保留 HTML 标题/图例/文字，又保留 JS 图表的 canvas 绘制内容。\n     */\n    function captureViaForeignObject(requestId, width, height) {\n        // 收集所有样式\n        var cssText = \'\';\n        for (var si = 0; si < document.styleSheets.length; si++) {\n            try {\n                var sheet = document.styleSheets[si];\n                for (var ri = 0; ri < sheet.cssRules.length; ri++) {\n                    cssText += sheet.cssRules[ri].cssText + \'\\n\';\n                }\n            } catch (_e) {\n                // 跨域样式表无法读取规则，跳过\n            }\n        }\n\n        var rootStyle = document.documentElement.getAttribute(\'style\') || \'\';\n        var darkClass = document.documentElement.classList.contains(\'dark\') ? \' class=&quot;dark&quot;\' : \'\';\n\n        // 克隆 DOM\n        var clone = root.cloneNode(true);\n\n        // 记录原始 canvas 的位置和内容，同时在克隆中替换为透明占位块\n        var originalCanvases = root.querySelectorAll(\'canvas\');\n        var clonedCanvases = clone.querySelectorAll(\'canvas\');\n        var canvasOverlays = [];\n        var rootRect = root.getBoundingClientRect();\n\n        for (var ci = 0; ci < originalCanvases.length; ci++) {\n            try {\n                var originalCanvas = originalCanvases[ci];\n                var clonedCanvas = clonedCanvases[ci];\n                if (originalCanvas &amp;&amp; clonedCanvas &amp;&amp; clonedCanvas.parentNode) {\n                    // 记录位置和 data URL 用于后续叠加\n                    var cRect = originalCanvas.getBoundingClientRect();\n                    canvasOverlays.push({\n                        dataUrl: originalCanvas.toDataURL(\'image/png\'),\n                        x: cRect.left - rootRect.left,\n                        y: cRect.top - rootRect.top,\n                        w: cRect.width,\n                        h: cRect.height\n                    });\n\n                    // 替换为同尺寸的透明占位 div（保持布局）\n                    var placeholder = document.createElement(\'div\');\n                    placeholder.style.width = originalCanvas.offsetWidth + \'px\';\n                    placeholder.style.height = originalCanvas.offsetHeight + \'px\';\n                    placeholder.style.display = \'block\';\n                    clonedCanvas.parentNode.replaceChild(placeholder, clonedCanvas);\n                }\n            } catch (_canvasErr) {\n                // canvas 可能被 tainted，跳过\n            }\n        }\n\n        // 移除 script 标签\n        var scripts = clone.querySelectorAll(\'script\');\n        for (var ssi = 0; ssi < scripts.length; ssi++) {\n            scripts[ssi].parentNode.removeChild(scripts[ssi]);\n        }\n\n        var serializer = new XMLSerializer();\n        var htmlContent = serializer.serializeToString(clone);\n\n        // 构建 SVG foreignObject\n        var svgNs = \'http://www.w3.org/2000/svg\';\n        var xhtmlNs = \'http://www.w3.org/1999/xhtml\';\n        var svgData = \'<svg xmlns=&quot;\' + svgNs + \'&quot; width=&quot;\' + width + \'&quot; height=&quot;\' + height + \'&quot;>\' +\n            \'<foreignObject width=&quot;100%&quot; height=&quot;100%&quot;>\' +\n            \'<html xmlns=&quot;\' + xhtmlNs + \'&quot;\' + darkClass + \' style=&quot;\' + rootStyle.replace(/&quot;/g, \'&amp;quot;\') + \'&quot;>\' +\n            \'<head><style>\' + cssText.split(\'</style>\').join(\'</st\' + \'yle>\') + \'</style></head>\' +\n            \'<body style=&quot;margin:0;padding:0;overflow:hidden;&quot;>\' + htmlContent + \'</body>\' +\n            \'</html>\' +\n            \'</foreignObject>\' +\n            \'</svg>\';\n\n        var img = new Image();\n        img.onload = function() {\n            try {\n                var padding = 24;\n                var canvas = document.createElement(\'canvas\');\n                var scale = height > 8000 ? 1 : 2;\n                var totalWidth = width + padding * 2;\n                var totalHeight = height + padding * 2;\n                canvas.width = totalWidth * scale;\n                canvas.height = totalHeight * scale;\n                var ctx = canvas.getContext(\'2d\');\n                ctx.scale(scale, scale);\n\n                // 背景色跟随主题\n                var isDark = document.documentElement.classList.contains(\'dark\');\n                ctx.fillStyle = isDark ? \'#1e1e1e\' : \'#ffffff\';\n                ctx.fillRect(0, 0, totalWidth, totalHeight);\n\n                // 绘制 foreignObject 渲染的 HTML/CSS 内容（带 padding 偏移）\n                ctx.drawImage(img, padding, padding, width, height);\n\n                // 叠加绘制 canvas 元素内容到对应位置（加 padding 偏移）\n                if (canvasOverlays.length > 0) {\n                    var overlaysLoaded = 0;\n                    var totalOverlays = canvasOverlays.length;\n\n                    for (var oi = 0; oi < totalOverlays; oi++) {\n                        (function(overlay) {\n                            var overlayImg = new Image();\n                            overlayImg.onload = function() {\n                                ctx.drawImage(overlayImg, overlay.x + padding, overlay.y + padding, overlay.w, overlay.h);\n                                overlaysLoaded++;\n                                if (overlaysLoaded === totalOverlays) {\n                                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                                }\n                            };\n                            overlayImg.onerror = function() {\n                                overlaysLoaded++;\n                                if (overlaysLoaded === totalOverlays) {\n                                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                                }\n                            };\n                            overlayImg.src = overlay.dataUrl;\n                        })(canvasOverlays[oi]);\n                    }\n                } else {\n                    sendCaptureResult(requestId, canvas.toDataURL(\'image/png\'), totalWidth, totalHeight);\n                }\n            } catch (canvasErr) {\n                sendCaptureError(requestId, \'Canvas render failed: \' + (canvasErr.message || canvasErr));\n            }\n        };\n        img.onerror = function() {\n            // foreignObject 渲染失败时，尝试纯 canvas 合成作为降级\n            var padding = 24;\n            var canvases = root.querySelectorAll(\'canvas\');\n            if (canvases.length > 0) {\n                try {\n                    var totalW = width + padding * 2;\n                    var totalH = height + padding * 2;\n                    var fallbackCanvas = document.createElement(\'canvas\');\n                    var scale = height > 8000 ? 1 : 2;\n                    fallbackCanvas.width = totalW * scale;\n                    fallbackCanvas.height = totalH * scale;\n                    var fCtx = fallbackCanvas.getContext(\'2d\');\n                    fCtx.scale(scale, scale);\n                    var isDark = document.documentElement.classList.contains(\'dark\');\n                    fCtx.fillStyle = isDark ? \'#1e1e1e\' : \'#ffffff\';\n                    fCtx.fillRect(0, 0, totalW, totalH);\n\n                    for (var fi = 0; fi < canvases.length; fi++) {\n                        var fc = canvases[fi];\n                        var fcRect = fc.getBoundingClientRect();\n                        fCtx.drawImage(fc, fcRect.left - rootRect.left + padding, fcRect.top - rootRect.top + padding, fcRect.width, fcRect.height);\n                    }\n\n                    sendCaptureResult(requestId, fallbackCanvas.toDataURL(\'image/png\'), totalW, totalH);\n                } catch (_fallbackErr) {\n                    sendCaptureError(requestId, \'SVG and canvas fallback both failed\');\n                }\n            } else {\n                sendCaptureError(requestId, \'SVG image load failed\');\n            }\n        };\n\n        var svgBase64 = btoa(unescape(encodeURIComponent(svgData)));\n        img.src = \'data:image/svg+xml;base64,\' + svgBase64;\n    }\n\n    // 通知宿主就绪\n    currentPhase = \'bootstrap\';\n    parent.postMessage({ type: \'widget:ready\' }, \'*\');\n})();';
var b=new Blob([s],{type:'text/javascript'});
var u=URL.createObjectURL(b);
var el=document.createElement('script');
var ttPolicy=null;

function initTrustedTypesPolicy(){
    if(ttPolicy||!window.trustedTypes||typeof window.trustedTypes.createPolicy!=='function') return;
    try{
        ttPolicy=window.trustedTypes.createPolicy('default',{
            createHTML:function(value){return value;},
            createScript:function(value){return value;},
            createScriptURL:function(value){return value;}
        });
    }catch(_error){
        ttPolicy=null;
    }
}

function toTrustedScriptURL(value){
    initTrustedTypesPolicy();
    if(ttPolicy&amp;&amp;typeof ttPolicy.createScriptURL==='function'){
        try{return ttPolicy.createScriptURL(value);}catch(_error){return value;}
    }
    return value;
}

function toTrustedScript(value){
    initTrustedTypesPolicy();
    if(ttPolicy&amp;&amp;typeof ttPolicy.createScript==='function'){
        try{return ttPolicy.createScript(value);}catch(_error){return value;}
    }
    return value;
}

function reportBootstrapFailure(reason, error){
    parent.postMessage({
        type:'widget:error',
        phase:'bootstrap',
        message:'Script load failed: '+(error&amp;&amp;error.message?error.message:'unknown'),
        detail:(error&amp;&amp;error.stack)||String(reason)||'',
        stack:error&amp;&amp;error.stack?String(error.stack):'',
        source:'bootstrap-script',
        runtime:{
            href:location.href,
            userAgent:navigator.userAgent,
            readyState:document.readyState,
            visibilityState:document.visibilityState,
            phase:'bootstrap',
            timestamp:new Date().toISOString()
        },
        extra:{
            bootstrapReason:String(reason||'')
        }
    },'*');
}

function tryEvalFallback(reason){
    try{
        // CSP 已允许 unsafe-eval，eval 不受 Trusted Types 约束
        (0,eval)(s);
        URL.revokeObjectURL(u);
        return true;
    }catch(error){
        reportBootstrapFailure(reason,error);
        URL.revokeObjectURL(u);
        return false;
    }
}

function tryInlineFallback(reason){
    try{
        var inline=document.createElement('script');
        var current=document.currentScript;
        var nonce=current&amp;&amp;typeof current.getAttribute==='function'?current.getAttribute('nonce'):'';
        if(nonce){
            inline.setAttribute('nonce',nonce);
        }
        inline.text=toTrustedScript(s);
        document.head.appendChild(inline);
        URL.revokeObjectURL(u);
        return true;
    }catch(error){
        // script.text 赋值被 Trusted Types 拦截时，使用 eval fallback
        return tryEvalFallback(reason);
    }
}

try {
    // 某些宿主启用 Trusted Types 时，script.src 需要 TrustedScriptURL
    el.src=toTrustedScriptURL(u);
} catch(assignError){
    tryInlineFallback(assignError&amp;&amp;assignError.message?assignError.message:assignError);
}

el.onload=function(){URL.revokeObjectURL(u)};
el.onerror=function(){
    tryInlineFallback('script-element-load-error');
};

try {
    document.head.appendChild(el);
} catch(appendError){
    tryInlineFallback(appendError&amp;&amp;appendError.message?appendError.message:appendError);
}</script>
</body>
</html>" title="HLS_GMAC_Authentication_Sequence" style="box-sizing: border-box; width: 786.391px; border: 0px; display: block; overflow: hidden; background: transparent; min-height: 0px; max-height: none; height: 531px;"></iframe>



HLS-GMAC 认证的核心是**双向挑战-应答**：双方各自生成随机挑战，使用 GMAC 和认证密钥计算应答值。密码（认证密钥）从不直接传输，即使截获通信也无法推导出密钥。

在 Suite 1/2 中，HLS 还可以使用 **ECDSA** 签名替代 GMAC，通过数字证书实现更强的身份验证和不可否认性。

------

### 五、安全策略与访问控制

DLMS 的安全不是一刀切的，而是通过 **Security Setup 对象** 和 **Association 对象** 灵活配置：

**安全策略 (Security Policy)** 控制每个操作的保护级别：

- **None**：明文，无认证
- **Authenticated**：仅认证（GMAC），不加密
- **Encrypted**：仅加密，不认证
- **Authenticated + Encrypted**：同时认证和加密（推荐）

这些策略可以分别应用于：

- **请求 (Request)**：客户端→电表方向的 Get/Set/Action 操作
- **响应 (Response)**：电表→客户端方向的数据返回

**访问控制**通过不同的 Association 实现分级：

| 客户端地址     | 权限级别 | 典型用途               |
| :------------- | :------- | :--------------------- |
| 16 (public)    | 公开读取 | 读取基本计量数据       |
| 1 (management) | 管理权限 | 配置参数、读取历史数据 |
| 其他           | 专用权限 | 固件升级、密钥管理等   |

------

### 六、安全性分析与实际注意事项

**强度分析：**

1. **AES-GCM 是经过充分验证的 AEAD 算法**，NIST SP 800-38D 标准化，在密码学社区有广泛信心
2. **Nonce 唯一性由 Invocation Counter 保证**，每次加密递增，有效防止 IV 重用攻击
3. **HLS 挑战-应答**避免了密码明文传输，抵抗中间人攻击
4. **Suite 1/2 的 ECDH 密钥协商**提供前向保密——即使长期密钥泄露，历史通信也无法解密

**潜在风险与注意事项：**

1. **Invocation Counter 同步问题**：如果计数器失同步（如电表掉电），可能导致通信中断。实践中需要通过公开客户端读取当前计数器值
2. **Suite 0 的局限性**：纯对称加密无法提供不可否认性和前向保密，密钥泄露后历史数据可被解密
3. **密钥管理是最大挑战**：每只电表有唯一密钥集，大规模部署时密钥的安全存储、分发、轮换极为关键。丢失密钥意味着电表不可访问
4. **默认密钥风险**：Gurux 等库使用默认密钥 `000102030405060708090A0B0C0D0E0F`，部署时必须替换
5. **硬件安全**：建议使用 HSM（硬件安全模块）或安全芯片（如 NXP EdgeLock SE05x）存储密钥，防止物理攻击
6. **Gurux 等开源实现**已被广泛使用，但需注意 AES 密钥长度配置（某些电表要求 AES-192 而非默认的 AES-128）

**实际部署建议：**

- 新项目应至少使用 Suite 1，利用非对称密码学增强安全性
- 密钥应通过 HSM 或安全元件 (Secure Element) 生成和存储
- 建立密钥轮换策略，定期更新加密密钥
- 使用 DLMS UA 的 Conformance Test Tool (CTT) 验证实现合规性

------

### 总结

DLMS/COSEM 的加密体系是一个**分层、可配置、渐进增强**的安全架构：

- **核心算法**：AES-GCM 提供 AEAD（认证加密），同时保证机密性和完整性
- **三个安全套件**：从纯对称 (Suite 0) 到混合加密 (Suite 1/2)，适配不同安全需求
- **多层密钥体系**：KEK → GUEK/GBEK/GAK → Dedicated/Ephemeral，层次清晰
- **灵活的认证机制**：LLS（简单密码）到 HLS-GMAC（挑战应答）再到 HLS-ECDSA（数字签名）
- **防重放保护**：Invocation Counter 确保消息新鲜性

这套体系在智能电表领域已经大规模部署验证，是目前 AMI（高级计量基础设施）领域最成熟的通信安全标准之一。