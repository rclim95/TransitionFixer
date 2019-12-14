; // Header
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )

LanguageNames=(Neutral=0x0000:MSG00000
              )

; // Messages
MessageId=0x0   
SymbolicName=MSG_INFO
Severity=Informational
Facility=Application
Language=Neutral
%1
.

MessageId=0x1   
SymbolicName=MSG_WARNING
Severity=Warning
Facility=Application
Language=Neutral
%1
.

MessageId=0x2   
SymbolicName=MSG_ERROR
Severity=Error
Facility=Application
Language=Neutral
%1
.

MessageId=0x3   
SymbolicName=MSG_SUCCESS
Severity=Success
Facility=Application
Language=Neutral
%1
.
