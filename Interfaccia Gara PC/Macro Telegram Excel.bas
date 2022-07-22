Attribute VB_Name = "Modulo2"
Sub InvioMessaggi()
Dim objRequest As Object
Dim strChatId As String
Dim strMessage As String
Dim strPostData As String
Dim strMessage2 As String



strChatId = -772651364

strMessage = " %F0%9F%9B%AB In linea  Paolo Pampana, Daniele Ghelardoni , Agrusa Alessandro."
strMessage2 = "%F0%9F%9A%A5 Rifornimento Farfara, Tomassini, Giannetti"
strPostData = "chat_id=" & strChatId & "&text=" & strMessage & vbCrLf & strMessage2


Set objRequest = CreateObject("MSXML2.XMLHTTP")

With objRequest
    .Open "POST", "https://api.telegram.org/bot5094549118:AAGjwkvIIlpJA_UE3c3VWviNvYDQjm4cKCo/sendMessage?", False
    .setRequestHeader "Content-Type", "application/x-www-form-urlencoded"
    .send (strPostData)

End With

End Sub

