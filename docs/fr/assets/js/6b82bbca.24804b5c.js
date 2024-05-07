"use strict";(self.webpackChunkkatalog_pages_2=self.webpackChunkkatalog_pages_2||[]).push([[137],{4801:(e,t,n)=>{n.r(t),n.d(t,{assets:()=>c,contentTitle:()=>s,default:()=>u,frontMatter:()=>o,metadata:()=>i,toc:()=>l});var r=n(4848),a=n(8453);const o={},s=void 0,i={id:"2 Development/Code-structure",title:"Code-structure",description:"Summary",source:"@site/docs/2 Development/Code-structure.md",sourceDirName:"2 Development",slug:"/2 Development/Code-structure",permalink:"/Katalog/fr/docs/2 Development/Code-structure",draft:!1,unlisted:!1,editUrl:"https://github.com/facebook/docusaurus/tree/main/packages/create-docusaurus/templates/shared/docs/2 Development/Code-structure.md",tags:[],version:"current",frontMatter:{},sidebar:"tutorialSidebar",previous:{title:"Build-from-sources",permalink:"/Katalog/fr/docs/2 Development/Build-from-sources"},next:{title:"Roadmap",permalink:"/Katalog/fr/docs/2 Development/Roadmap"}},c={},l=[{value:"Summary",id:"summary",level:2},{value:"Model and file structure",id:"model-and-file-structure",level:2},{value:"Code practice",id:"code-practice",level:2}];function d(e){const t={h2:"h2",li:"li",p:"p",ul:"ul",...(0,a.R)(),...e.components};return(0,r.jsxs)(r.Fragment,{children:[(0,r.jsx)(t.h2,{id:"summary",children:"Summary"}),"\n",(0,r.jsx)(t.p,{children:"This page provides information about how the source code is organized and any common practice used to facilitate its understanding, maintenance and evolution."}),"\n",(0,r.jsx)(t.h2,{id:"model-and-file-structure",children:"Model and file structure"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsx)(t.li,{children:"Each tab / screen of Katalog is managed in a different cpp file, belonging to the mainwindow code."}),"\n",(0,r.jsx)(t.li,{children:"Each object requiring a data model to populate views has its class/header files: catalog, storage"}),"\n"]}),"\n",(0,r.jsx)(t.h2,{id:"code-practice",children:"Code practice"}),"\n",(0,r.jsxs)(t.ul,{children:["\n",(0,r.jsx)(t.li,{children:"Comments, comments, comments."}),"\n",(0,r.jsx)(t.li,{children:"variables:  first word lower cap, all other starting with capital letter:    thisIsAVariableName."}),"\n",(0,r.jsx)(t.li,{children:"database fields: to help with compatibility between SQLite and Postgres, fields are named in lower case, words separated by underscore: this_is_a_fied_name"}),"\n"]})]})}function u(e={}){const{wrapper:t}={...(0,a.R)(),...e.components};return t?(0,r.jsx)(t,{...e,children:(0,r.jsx)(d,{...e})}):d(e)}},8453:(e,t,n)=>{n.d(t,{R:()=>s,x:()=>i});var r=n(6540);const a={},o=r.createContext(a);function s(e){const t=r.useContext(o);return r.useMemo((function(){return"function"==typeof e?e(t):{...t,...e}}),[t,e])}function i(e){let t;return t=e.disableParentContext?"function"==typeof e.components?e.components(a):e.components||a:s(e.components),r.createElement(o.Provider,{value:t},e.children)}}}]);